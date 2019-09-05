/**@file Library to interface with alternate boot variable

  Copyright (C) Microsoft Corporation. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Uefi.h>
#include <Library/DebugLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/MuUefiVersionLib.h>
#include "MsAltBootVariables.h"

/**
  Clears the Alternate boot flag
**/
VOID
EFIAPI
ClearAltBoot (
  VOID
)
{
  EFI_STATUS              Status;
  UINTN                   AltBootFlag, FlagSize;
  UINT32                  Attributes;

  FlagSize = sizeof(UINTN);
  Status = gRT->GetVariable(kszAltBootFlagVariableName,
                  &gAltBootGuid,
                  &Attributes,
                  &FlagSize,
                  &AltBootFlag);

  //Alt boot flag exists and value is 1, so set it back to 0
  if (Status == EFI_SUCCESS && AltBootFlag == 1)
  {
    AltBootFlag = 0;
    Status = gRT->SetVariable(kszAltBootFlagVariableName,
                    &gAltBootGuid,
                    Attributes,
                    FlagSize,
                    &AltBootFlag);
    if (EFI_ERROR(Status) && (Status != EFI_NOT_FOUND))
    {
      DEBUG((DEBUG_ERROR, "%a - Failed to clear Alternate Boot Flag! %r\n", __FUNCTION__, Status));
      ASSERT(FALSE);
    }
  }
}

/**
  Set the Alternate boot flag

  @retval  EFI_SUCCESS  Set AltBoot successfully
  @retval  !EFI_SUCCESS Failed to set AltBoot
**/
EFI_STATUS
EFIAPI
SetAltBoot (
  VOID
)
{
  EFI_STATUS              Status;
  UINTN                   AltBootFlag, FlagSize;
  FlagSize = sizeof(UINTN);

  Status = gRT->GetVariable(
                  kszAltBootFlagVariableName,
                  &gAltBootGuid,
                  0,
                  &FlagSize,
                  &AltBootFlag
                );
  if (EFI_ERROR(Status) && Status != EFI_NOT_FOUND) { // some error besides EFI_NOT_FOUND
    DEBUG((DEBUG_ERROR, "%a - Failed to retrieve alt boot variable! %r\n", __FUNCTION__, Status));
  }
  if (Status == EFI_SUCCESS && AltBootFlag == 0) {
    AltBootFlag = 1;
    Status = gRT->SetVariable(
                  kszAltBootFlagVariableName,
                  &gAltBootGuid,
                  EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_RUNTIME_ACCESS,
                  FlagSize,
                  &AltBootFlag
                );
    if (EFI_ERROR(Status)) {
        DEBUG((DEBUG_ERROR, "%a - Failed to set alt boot variable! %r\n",__FUNCTION__, Status));
    }
  }
  return Status;
}
