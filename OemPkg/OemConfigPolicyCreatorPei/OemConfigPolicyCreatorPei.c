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

typedef enum {
  KNOB_PowerOnPort0,
  KNOB_MAX
} KNOB;

typedef struct {
  INTN    GetCount;
  INTN    SetCount;
} KNOB_STATISTICS;

typedef BOOLEAN (KNOB_VALIDATION_FN)(
  CONST VOID *
  );

typedef struct {
  KNOB                  Knob;
  CONST VOID            *DefaultValueAddress;
  VOID                  *CacheValueAddress;
  UINTN                 ValueSize;
  CONST CHAR8           *Name;
  UINTN                 NameSize;
  EFI_GUID              VendorNamespace;
  INTN                  Attributes;
  KNOB_STATISTICS       Statistics;
  KNOB_VALIDATION_FN    *Validator;
} KNOB_DATA;

extern KNOB_DATA  gKnobData[];
extern UINTN      gNumKnobs;

typedef struct {
  UINTN       NameSize;
  UINTN       DataSize;
  EFI_GUID    NamespaceGuid;
} CONF_POLICY_ENTRY;

/**
  Helper function to create config policy.

  @param[out]      ConfPolicy     Pointer to uninitialized config policy
  @param[out]      ConfPolicySize Pointer to size of created ConfPolicy

  @retval EFI_SUCCESS           The configuration is translated to policy successfully.
  @retval EFI_OUT_OF_RESOURCES  Memory allocation failed.
  @retval EFI_NOT_READY         Variable Services were not found.
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
  // NumKnobs is total knobs minus dummy KNOB_MAX at end
  UINTN                            NumKnobs = gNumKnobs - 1;
  UINTN                            NeededSize = 0;
  UINTN                            Offset     = 0;
  EFI_PEI_READ_ONLY_VARIABLE2_PPI  *PPIVariableServices;
  UINTN                            VarSize;
  UINTN                            UnicodeNameSize = 0;
  CHAR16                           *UnicodeName = NULL;

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
  for (i = 0; i < NumKnobs; i++) {
    // NeededSize is sum of NameSize, ValueSize, size of the guid,
    // size of the NameSize field, size of the DataSize field
    NeededSize += gKnobData[i].NameSize + gKnobData[i].ValueSize + sizeof (EFI_GUID) + sizeof (UINT32) + sizeof (UINT32);
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
  for (i = 0; i < NumKnobs; i++) {
    ((CONF_POLICY_ENTRY *)((CHAR8 *)*ConfPolicy + Offset))->NameSize      = gKnobData[i].NameSize;
    ((CONF_POLICY_ENTRY *)((CHAR8 *)*ConfPolicy + Offset))->DataSize      = gKnobData[i].ValueSize;
    ((CONF_POLICY_ENTRY *)((CHAR8 *)*ConfPolicy + Offset))->NamespaceGuid = gKnobData[i].VendorNamespace;
    Offset                                                               += sizeof (CONF_POLICY_ENTRY);

    // convert ASCII name to Unicode
    UnicodeNameSize = gKnobData[i].NameSize * 2;
    UnicodeName = AllocatePool (UnicodeNameSize);
    if (UnicodeName == NULL) {
      DEBUG((DEBUG_ERROR, "%a failed to allocate memory for unicode name string!\n", __FUNCTION__));
      ASSERT (FALSE);
      FreePool (*ConfPolicy);
      *ConfPolicySize = 0;
      return EFI_OUT_OF_RESOURCES;
    }

    AsciiStrToUnicodeStrS (gKnobData[i].Name, UnicodeName, gKnobData[i].NameSize);

    // check if value has been overridden in variable storage
    VarSize = gKnobData[i].ValueSize;
    Status  = PPIVariableServices->GetVariable (
                                     PPIVariableServices,
                                     UnicodeName,
                                     &gKnobData[i].VendorNamespace,
                                     NULL,
                                     &VarSize,
                                     gKnobData[i].CacheValueAddress
                                     );

    // if variable services fails other than failing to find the variable (it was not overidden)
    // or a size mismatch (stale data from a previous definition of the knob), we should fail
    // as we may be missing overidden knobs
    if (EFI_ERROR (Status) && (Status != EFI_NOT_FOUND) && (Status != EFI_BUFFER_TOO_SMALL)) {
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

    CopyMem ((CHAR8 *)*ConfPolicy + Offset, UnicodeName, UnicodeNameSize);
    Offset += UnicodeNameSize;

    CopyMem ((CHAR8 *)*ConfPolicy + Offset, gKnobData[i].CacheValueAddress, gKnobData[i].ValueSize);
    Offset += gKnobData[i].ValueSize;

    FreePool (UnicodeName);
  }

  DEBUG ((DEBUG_ERROR, "%a ConfPolicy: %p *ConfPolicy: %p\n", __FUNCTION__, ConfPolicy, *ConfPolicy));

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
