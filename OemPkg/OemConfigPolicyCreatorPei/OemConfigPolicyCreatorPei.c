/** @file
  This module receives the static platform data for a platform and creates a config policy. It is intended to run
  before a platform level Silicon Policy Creator that maps this config policy to silicon policy.

  Copyright (c) Microsoft Corporation.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Uefi.h>

#include <Ppi/ReadOnlyVariable2.h>
#include <Ppi/Policy.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PeiServicesLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/ConfigVariableListLib.h>
#include <Library/ConfigKnobShimLib.h>
#include <ConfigStdStructDefs.h>

extern KNOB_DATA  gKnobData[];
extern UINTN      gNumKnobs;

/**
  Helper function to create config policy.

  @param[out]      ConfPolicy     Pointer to uninitialized config policy
  @param[out]      ConfPolicySize Pointer to size of created ConfPolicy

  @retval EFI_SUCCESS           The configuration is translated to policy successfully.
  @retval EFI_OUT_OF_RESOURCES  Memory allocation failed.
  @retval EFI_NOT_READY         Variable Services were not found.
  @retval EFI_ABORTED           Creating variable list failed.
  @retval Others                Other errors occurred when getting GFX policy.
**/
STATIC
EFI_STATUS
CreateConfPolicy (
  OUT  VOID    **ConfPolicy,
  OUT   UINT16  *ConfPolicySize
  )
{
  EFI_STATUS  Status;
  UINT32                           i;
  UINTN                            NeededSize = 0;
  UINTN                            Offset     = 0;
  EFI_PEI_READ_ONLY_VARIABLE2_PPI  *PPIVariableServices;
  UINTN                            UnicodeNameSize = 0;
  // get a buffer of the max name size
  CHAR16                           UnicodeName[CONF_VAR_NAME_LEN];
  CONFIG_VAR_LIST_ENTRY            VarListEntry;
  UINTN                            VarListSize;
  VOID                             *ConfListPtr;

  Status = PeiServicesLocatePpi (
             &gEfiPeiReadOnlyVariable2PpiGuid,
             0,
             NULL,
             (VOID **)&PPIVariableServices
             );

  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a failed to locate variable services!\n", __FUNCTION__));
    ASSERT (FALSE);
    return EFI_NOT_READY;
  }

  // first figure out how much space we need to allocate for the ConfPolicy
  for (i = 0; i < gNumKnobs; i++) {
    // the var list will use the Unicode version of the name, gKnobData has the ASCII version
    NeededSize += VAR_LIST_SIZE(gKnobData[i].NameSize * 2, gKnobData[i].ValueSize);
    DEBUG ((DEBUG_ERROR, "OSDDEBUG NeededSize: %x\n", NeededSize));
  }

  if (NeededSize > MAX_UINT16) {
    DEBUG((DEBUG_ERROR, "%a config is greater than 64k! Too large for what policy service supports\n", __FUNCTION__));
    ASSERT (FALSE);
    return EFI_UNSUPPORTED;
  }

  *ConfPolicy = AllocatePool (NeededSize);

  if (*ConfPolicy == NULL) {
    DEBUG ((DEBUG_ERROR, "%a failed to allocate Conf Policy memory!\n", __FUNCTION__));
    ASSERT (FALSE);
    return EFI_OUT_OF_RESOURCES;
  } 

  *ConfPolicySize = (UINT16)NeededSize;

  // now go through and populate the Conf Policy
  for (i = 0; i < gNumKnobs; i++) {
    UnicodeNameSize = gKnobData[i].NameSize * 2;
    AsciiStrToUnicodeStrS (gKnobData[i].Name, UnicodeName, gKnobData[i].NameSize);

    VarListEntry.Name = UnicodeName;
    VarListEntry.Guid = gKnobData[i].VendorNamespace;
    // hardcoded for now
    VarListEntry.Attributes = 7;
    VarListEntry.Data = gKnobData[i].CacheValueAddress;
    VarListEntry.DataSize = gKnobData[i].ValueSize;

    DUMP_HEX(DEBUG_ERROR, 0, gKnobData[i].CacheValueAddress, sizeof(BOOLEAN), "OSDDEBUG: ");

    // check if value has been overridden in variable storage
    Status = GetConfigKnobOverride (&gKnobData[i].VendorNamespace,
                                    UnicodeName,
                                    gKnobData[i].CacheValueAddress,
                                    gKnobData[i].ValueSize);

    // if variable services fails other than failing to find the variable (it was not overidden)
    // or a size mismatch (stale data from a previous definition of the knob), we should fail
    // as we may be missing overidden knobs
    if (EFI_ERROR (Status) && (Status != EFI_NOT_FOUND) && (Status != EFI_BAD_BUFFER_SIZE)) {
      DEBUG ((
        DEBUG_ERROR,
        "%a variable services failed to find %a with status (%r)\n",
        __FUNCTION__,
        gKnobData[i].Name,
        Status
        ));
      ASSERT (FALSE);
      FreePool (*ConfPolicy);
      *ConfPolicySize = 0;
      return Status;
    }

    // Validate the value from flash meets the constraints of the knob
    if (Status == EFI_SUCCESS && gKnobData[i].Validator != NULL) {
      if (!gKnobData[i].Validator (gKnobData[i].CacheValueAddress)) {
        // If it doesn't, we will set the value to the default value
        DEBUG ((DEBUG_ERROR, "Config knob %a failed validation!\n", gKnobData[i].Name));
        CopyMem (gKnobData[i].CacheValueAddress, gKnobData[i].DefaultValueAddress, gKnobData[i].ValueSize);
      }
    }

    DUMP_HEX(DEBUG_ERROR, 0, gKnobData[i].CacheValueAddress, sizeof(BOOLEAN), "OSDDEBUG: ");
    ConfListPtr = ((UINT8 *)*ConfPolicy) + Offset;

    Status = ConvertVariableEntryToVariableList (&VarListEntry, &ConfListPtr, &VarListSize);

    if (EFI_ERROR (Status)) {
      DEBUG((DEBUG_ERROR, "%a failed to convert variable entry to var list!\n", __FUNCTION__));
      ASSERT (FALSE);
      FreePool (*ConfPolicy);
      return EFI_ABORTED;
    }

    Offset += VarListSize;
  }

  DUMP_HEX(DEBUG_ERROR, 0, *ConfPolicy, Offset, "OSDDEBUG: ");

  if (Offset != NeededSize) {
    // oops we messed up the math, may have corrupted memory...
    DEBUG((DEBUG_ERROR, "%a expected ConfPolicy size %x does not match actual size %x!\n", __FUNCTION__, NeededSize, Offset));
    ASSERT (Offset == NeededSize);
    FreePool (*ConfPolicy);
    return EFI_ABORTED;
  }

  DEBUG ((DEBUG_ERROR, "%a ConfPolicy: %p *ConfPolicy: %p ConfVarListPtr: %p &ConfListPtr: %p\n", __FUNCTION__, ConfPolicy, *ConfPolicy, ConfListPtr, &ConfListPtr));

  return EFI_SUCCESS;
}

/**
  Module entry point that will check configuration data and publish them to policy database.

  @param FileHandle                     The image handle.
  @param PeiServices                    The PEI services table.

  @retval Status                        From internal routine or boot object, should not fail
**/
EFI_STATUS
EFIAPI
OemConfigPolicyCreatorPeiEntry (
  IN EFI_PEI_FILE_HANDLE     FileHandle,
  IN CONST EFI_PEI_SERVICES  **PeiServices
  )
{
  EFI_STATUS  Status;
  POLICY_PPI  *PolPpi        = NULL;
  VOID        *ConfPolicy    = NULL;
  UINT16       ConfPolicySize = 0;

  DEBUG ((DEBUG_INFO, "%a - Entry.\n", __FUNCTION__));

  // First locate policy ppi.
  Status = PeiServicesLocatePpi (&gPeiPolicyPpiGuid, 0, NULL, (VOID *)&PolPpi);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a Failed to locate Policy PPI - %r\n", __FUNCTION__, Status));
    ASSERT (FALSE);
    return Status;
  }

  // Oem can choose to do any Oem specific things to config here such as enforcing static only config or
  // selecting a configuration profile based on some criteria

  Status = CreateConfPolicy (&ConfPolicy, &ConfPolicySize);

  DEBUG ((DEBUG_ERROR, "%a ConfPolicy: %p Size: %d\n", __FUNCTION__, ConfPolicy, ConfPolicySize));

  if (EFI_ERROR (Status) || (ConfPolicy == NULL) || (ConfPolicySize == 0)) {
    DEBUG ((DEBUG_ERROR, "%a CreateConfPolicy failed! Status (%r)\n", __FUNCTION__, Status));
    ASSERT (FALSE);
    return Status;
  }

  DEBUG ((DEBUG_ERROR, "OSDDEBUG 1\n"));

  // Publish immutable config policy
  // Policy Service will publish the  gOemConfigPolicyPpiGuid so that the Silicon Policy Creator can consume our
  // Config Policy and map it to Silicon Policies
  Status = PolPpi->SetPolicy (&gOemConfigPolicyGuid, POLICY_ATTRIBUTE_FINALIZED, ConfPolicy, ConfPolicySize);

  DEBUG ((DEBUG_ERROR, "OSDDEBUG 2\n"));

  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a Failed to set config policy! Status (%r)\n", __FUNCTION__, Status));
    ASSERT (FALSE);
  }

  // Policy Service copies the policy, so we can free this memory
  FreePool (ConfPolicy);

  DEBUG ((DEBUG_ERROR, "OSDDEBUG 3\n"));

  return Status;
}
