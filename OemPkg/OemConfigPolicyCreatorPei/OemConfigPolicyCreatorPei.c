/** @file
  This module receives the static platform data for a platform and creates a config policy. It is intended to run
  before a platform level Silicon Policy Creator that maps this config policy to silicon policy.

  Copyright (c) Microsoft Corporation.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Uefi.h>

#include <Ppi/Policy.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PeiServicesLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Guid/VariableFormat.h>
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
  OUT  VOID     **ConfPolicy,
  OUT   UINT16  *ConfPolicySize
  )
{
  EFI_STATUS                       Status;
  UINT32                           i;
  UINTN                            NeededSize = 0;
  UINTN                            Offset     = 0;
  CHAR16                           UnicodeName[CONF_VAR_NAME_LEN]; // get a buffer of the max name size
  CONFIG_VAR_LIST_ENTRY            VarListEntry;
  UINTN                            VarListSize;
  VOID                             *ConfListPtr;

  // first figure out how much space we need to allocate for the ConfPolicy
  for (i = 0; i < gNumKnobs; i++) {
    // the var list will use the Unicode version of the name, gKnobData has the ASCII version
    NeededSize += VAR_LIST_SIZE (gKnobData[i].NameSize * 2, gKnobData[i].ValueSize);
    DEBUG ((DEBUG_ERROR, "OSDDEBUG NeededSize: %x\n", NeededSize));
  }

  if (NeededSize > MAX_UINT16) {
    DEBUG ((DEBUG_ERROR, "%a config is greater than 64k! Too large for what policy service supports\n", __FUNCTION__));
    ASSERT (FALSE);
    Status = EFI_UNSUPPORTED;
    goto CreatePolicyExit;
  }

  *ConfPolicy = AllocatePool (NeededSize);

  if (*ConfPolicy == NULL) {
    DEBUG ((DEBUG_ERROR, "%a failed to allocate Conf Policy memory!\n", __FUNCTION__));
    ASSERT (FALSE);
    Status = EFI_OUT_OF_RESOURCES;
    goto CreatePolicyExit;
  }

  *ConfPolicySize = (UINT16)NeededSize;

  // now go through and populate the Conf Policy
  for (i = 0; i < gNumKnobs; i++) {
    AsciiStrToUnicodeStrS (gKnobData[i].Name, UnicodeName, gKnobData[i].NameSize);

    VarListEntry.Name = UnicodeName;
    VarListEntry.Guid = gKnobData[i].VendorNamespace;
    // hardcoded for now
    VarListEntry.Attributes = VARIABLE_ATTRIBUTE_NV_BS_RT;
    VarListEntry.Data       = gKnobData[i].CacheValueAddress;
    if (gKnobData[i].ValueSize > MAX_UINT32) {
      // we overflowed, better bail out
      DEBUG ((
        DEBUG_ERROR,
        "%a knob %s had too large of a value size: 0x%x\n",
        __FUNCTION__,
        gKnobData[i].Name,
        gKnobData[i].ValueSize
        ));
      ASSERT (FALSE);
      Status = EFI_INVALID_PARAMETER;
      goto CreatePolicyExit;
    }

    VarListEntry.DataSize = (UINT32)gKnobData[i].ValueSize;

    // check if value has been overridden in variable storage
    Status = GetConfigKnobOverride (
               &gKnobData[i].VendorNamespace,
               UnicodeName,
               gKnobData[i].CacheValueAddress,
               gKnobData[i].ValueSize
               );

    // if variable services fails other than failing to find the variable (it was not overridden)
    // or a size mismatch (stale data from a previous definition of the knob), we should fail
    // as we may be missing overridden knobs
    if (EFI_ERROR (Status) && (Status != EFI_NOT_FOUND) && (Status != EFI_BAD_BUFFER_SIZE)) {
      DEBUG ((
        DEBUG_ERROR,
        "%a variable services failed to find %a with status (%r)\n",
        __FUNCTION__,
        gKnobData[i].Name,
        Status
        ));
      ASSERT (FALSE);
      goto CreatePolicyExit;
    }

    // Validate the value from flash meets the constraints of the knob
    if ((Status == EFI_SUCCESS) && (gKnobData[i].Validator != NULL)) {
      if (!gKnobData[i].Validator (gKnobData[i].CacheValueAddress)) {
        // If it doesn't, we will set the value to the default value
        DEBUG ((DEBUG_ERROR, "Config knob %a failed validation!\n", gKnobData[i].Name));
        CopyMem (gKnobData[i].CacheValueAddress, gKnobData[i].DefaultValueAddress, gKnobData[i].ValueSize);
      }
    }

    ConfListPtr = ((UINT8 *)*ConfPolicy) + Offset;

    VarListSize = NeededSize - Offset;
    Status      = ConvertVariableEntryToVariableList (&VarListEntry, ConfListPtr, &VarListSize);

    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "%a failed to convert variable entry to var list! - %r\n", __FUNCTION__, Status));
      ASSERT (FALSE);
      goto CreatePolicyExit;
    }

    Offset += VarListSize;
  }

  if (Offset != NeededSize) {
    // oops we messed up the math, may have corrupted memory...
    DEBUG ((DEBUG_ERROR, "%a expected ConfPolicy size %x does not match actual size %x!\n", __FUNCTION__, NeededSize, Offset));
    ASSERT (Offset == NeededSize);
    Status = EFI_ABORTED;
    goto CreatePolicyExit;
  }

CreatePolicyExit:
  if (EFI_ERROR (Status)) {
    *ConfPolicySize = 0;

    if (*ConfPolicy != NULL) {
      FreePool (*ConfPolicy);
    }
  }

  return Status;
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
  UINT16      ConfPolicySize = 0;

  DEBUG ((DEBUG_INFO, "%a - Entry.\n", __FUNCTION__));

  // First locate policy ppi.
  Status = PeiServicesLocatePpi (&gPeiPolicyPpiGuid, 0, NULL, (VOID *)&PolPpi);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a Failed to locate Policy PPI - %r\n", __FUNCTION__, Status));
    ASSERT (FALSE);
    goto Exit;
  }

  // Oem can choose to do any Oem specific things to config here such as enforcing static only config or
  // selecting a configuration profile based on some criteria

  Status = CreateConfPolicy (&ConfPolicy, &ConfPolicySize);

  if (EFI_ERROR (Status) || (ConfPolicy == NULL) || (ConfPolicySize == 0)) {
    DEBUG ((DEBUG_ERROR, "%a CreateConfPolicy failed! Status (%r)\n", __FUNCTION__, Status));
    ASSERT (FALSE);
    goto Exit;
  }

  // Publish immutable config policy
  // Policy Service will publish the gOemConfigPolicyGuid so that the Silicon Policy Creator can consume our
  // Config Policy and map it to Silicon Policies
  Status = PolPpi->SetPolicy (&gOemConfigPolicyGuid, POLICY_ATTRIBUTE_FINALIZED, ConfPolicy, ConfPolicySize);

  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a Failed to set config policy! Status (%r)\n", __FUNCTION__, Status));
    ASSERT (FALSE);
    goto Exit;
  }

Exit:
  // Policy Service copies the policy, so we can free this memory
  if (ConfPolicy != NULL) {
    FreePool (ConfPolicy);
  }

  // return success even in failure scenarios as returning a failure from an entry point can cause the image to be
  // unloaded. In this way depend modules can come up and run their own failure scenarios for not finding the config
  // policy
  return EFI_SUCCESS;
}
