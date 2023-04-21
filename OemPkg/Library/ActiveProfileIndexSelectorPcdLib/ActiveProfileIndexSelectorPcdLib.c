/** @file ActiveProfileIndexSelectorPcdLib.c
  PCD instance of ActiveProfileIndexSelectorLib. It is expected that the OEM/Platform
  will override this library to query the current boot active profile index from the
  proper source of truth.

  This library reads gOemPkgTokenSpaceGuid.PcdActiveProfileIndex and returns the active
  profile index.

  Copyright (c) Microsoft Corporation.
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/
#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/PcdLib.h>
#include <Library/ActiveProfileIndexSelectorLib.h>

/**
  Return which profile is the active profile for this boot.
  This function validates the profile GUID is valid.

  @param[out] ActiveProfileIndex  The index for the active profile. A value of MAX_UINT32, when combined with a return
                                  value of EFI_SUCCESS, indicates that the default profile has been chosen. If the
                                  return value is not EFI_SUCCESS, the value of ActiveProfileIndex shall not be updated.

  @retval EFI_INVALID_PARAMETER   Input argument is null.
  @retval EFI_NO_RESPONSE         The source of truth for profile selection has returned a garbage value or not replied.
  @retval EFI_SUCCESS             The operation succeeds and ActiveProfileIndex contains the valid active profile
                                  index for this boot.
**/
EFI_STATUS
EFIAPI
GetActiveProfileIndex (
  OUT UINT32  *ActiveProfileIndex
  )
{
  if (ActiveProfileIndex == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  // Just return what the PCD has, the caller should validate it
  *ActiveProfileIndex = FixedPcdGet32 (PcdActiveProfileIndex);

  return EFI_SUCCESS;
}
