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
#include <Library/SafeIntLib.h>
#include <Library/ActiveProfileIndexSelectorLib.h>
#include <ConfigStdStructDefs.h>

// these externs are provided by PlatformConfigDataLib
extern KNOB_DATA  gKnobData[];
extern UINTN      gNumKnobs;
// number of profile overrides (i.e. into gProfileData)
// this does not count the generic profile, which is not
// in gProfileData, but rather in gKnobData's defaults
extern UINTN    gNumProfiles;
extern PROFILE  gProfileData[];

/**
  Helper function to apply profile overrides. This function is only
  called if the active profile index has been validated to be within
  the acceptable range.
  @param[in]  ActiveProfileIndex The validated index into gProfileData
**/
STATIC
VOID
ApplyProfileOverrides (
  UINT32  ActiveProfileIndex
  )
{
  PROFILE  *ActiveProfile = &gProfileData[ActiveProfileIndex];
  UINTN    i;
  UINTN    Knob;

  for (i = 0; i < ActiveProfile->OverrideCount; i++) {
    Knob = ActiveProfile->Overrides[i].Knob;
    if ((Knob >= gNumKnobs) || (Knob != gKnobData[Knob].Knob)) {
      // something bad happened in the autogeneration, knobs are
      // not ordered. Don't muck about finding the misordered knobs
      // fail
      DEBUG ((
        DEBUG_ERROR,
        "%a knob autogeneration out of order, can't finish applying profile, defaulting to generic profile\n",
        __FUNCTION__
        ));

      // we may have failed in the middle of applying the profile, so we need to
      // write the default value for each knob we may have passed to its cache address so we can
      // have a clean generic profile. We didn't apply the knob for this value of i, so decrement
      // before we start
      i--;
      for ( ; i > 0; i--) {
        Knob = ActiveProfile->Overrides[i].Knob;
        CopyMem (gKnobData[Knob].CacheValueAddress, gKnobData[Knob].DefaultValueAddress, gKnobData[Knob].ValueSize);
      }

      // do this once more when i == 0. To keep signed/unsigned comparisons issues in check, i is not signed
      // otherwise we'd do >= 0 in the loop
      Knob = ActiveProfile->Overrides[i].Knob;
      CopyMem (gKnobData[Knob].CacheValueAddress, gKnobData[Knob].DefaultValueAddress, gKnobData[Knob].ValueSize);

      ASSERT (FALSE);
      return;
    }

    CopyMem (gKnobData[Knob].CacheValueAddress, ActiveProfile->Overrides[i].Value, gKnobData[Knob].ValueSize);
  }
}

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
  EFI_STATUS             Status;
  UINT32                 i;
  UINT32                 NeededSize = 0;
  UINT32                 Offset     = 0;
  CHAR16                 UnicodeName[CONF_VAR_NAME_LEN];           // get a buffer of the max name size
  CONFIG_VAR_LIST_ENTRY  VarListEntry;
  UINTN                  VarListSize;
  VOID                   *ConfListPtr;
  UINT32                 UnicodeNameSize;
  UINT32                 TmpNeededSize;
  UINT32                 ActiveProfileIndex;

  // first figure out how much space we need to allocate for the ConfPolicy
  for (i = 0; i < gNumKnobs; i++) {
    Status = (EFI_STATUS)SafeUint32Mult (gKnobData[i].NameSize, 2, &UnicodeNameSize);
    if (EFI_ERROR (Status)) {
      // we overflowed
      DEBUG ((DEBUG_ERROR, "%a config knob has too long a name! Size: 0x%x\n", __FUNCTION__, gKnobData[i].NameSize * 2));
      ASSERT (FALSE);
      Status = EFI_UNSUPPORTED;
      goto CreatePolicyExit;
    }

    // the var list will use the Unicode version of the name, gKnobData has the ASCII version
    Status = GetVarListSize (UnicodeNameSize, gKnobData[i].ValueSize, &TmpNeededSize);
    if (EFI_ERROR (Status)) {
      // we overflowed
      DEBUG ((DEBUG_ERROR, "%a Config var list is too large!\n", __FUNCTION__));
      ASSERT (FALSE);
      Status = EFI_UNSUPPORTED;
      goto CreatePolicyExit;
    }

    Status = (EFI_STATUS)SafeUint32Add (NeededSize, TmpNeededSize, &NeededSize);
    if (EFI_ERROR (Status)) {
      // we overflowed
      DEBUG ((DEBUG_ERROR, "%a config exceeds max size!\n", __FUNCTION__));
      ASSERT (FALSE);
      Status = EFI_UNSUPPORTED;
      goto CreatePolicyExit;
    }
  }

  Status = (EFI_STATUS)SafeUint32ToUint16 (NeededSize, ConfPolicySize);
  // validate we won't overflow the maximum size allowed for policy service. In addition, don't
  // overflow the DataSize (UINT32) of VarListEntry
  if (EFI_ERROR (Status)) {
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

  // before we get potential overrides for the policy, we need to figure out which
  // profile will be our active one for this boot and apply any overrides from it
  // to the cache
  Status = GetActiveProfileIndex (&ActiveProfileIndex);
  if (EFI_ERROR (Status)) {
    // if we fail to get the active profile index, default to the generic profile
    DEBUG ((DEBUG_ERROR, "%a failed to get active profile index! Defaulting to generic profile\n", __FUNCTION__));
    ActiveProfileIndex = MAX_UINT32;
  }

  if ((ActiveProfileIndex >= 0) && (ActiveProfileIndex < gNumProfiles)) {
    // if ActiveProfileIndex == MAX_UINT32, we are using the generic profile and don't
    // look for any profile overrides. Otherwise, ensure that the active profile index
    // is valid, otherwise use the generic profile. If it is valid, apply those overrides.
    ApplyProfileOverrides (ActiveProfileIndex);
  } else if (ActiveProfileIndex != MAX_UINT32) {
    DEBUG ((
      DEBUG_ERROR,
      "%a bad value of ActiveProfileIndex returned: %d, defaulting to generic profile\n",
      __FUNCTION__,
      ActiveProfileIndex
      ));
    ActiveProfileIndex = MAX_UINT32;
  }

  // now go through and populate the Conf Policy
  for (i = 0; i < gNumKnobs; i++) {
    AsciiStrToUnicodeStrS (gKnobData[i].Name, UnicodeName, gKnobData[i].NameSize);

    VarListEntry.Name = UnicodeName;
    VarListEntry.Guid = gKnobData[i].VendorNamespace;
    // hardcoded for now
    VarListEntry.Attributes = VARIABLE_ATTRIBUTE_NV_BS_RT;
    VarListEntry.Data       = gKnobData[i].CacheValueAddress;
    // this is validated not to overflow above where we ensure the entire config
    // NeededSize is not greater than MAX_UINT16 and NeededSize is takes each knob's
    // ValueSize into consideration
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

    VarListSize = (UINTN)NeededSize - (UINTN)Offset;
    Status      = ConvertVariableEntryToVariableList (&VarListEntry, ConfListPtr, &VarListSize);

    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "%a failed to convert variable entry to var list! - %r\n", __FUNCTION__, Status));
      ASSERT (FALSE);
      goto CreatePolicyExit;
    }

    Offset += (UINT32)VarListSize;
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
  // Policy Service will receive gOemConfigPolicyGuid and publish it as a PPI so that the Silicon Policy Creator can
  // have a depex on it and map it to Silicon Policies
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
