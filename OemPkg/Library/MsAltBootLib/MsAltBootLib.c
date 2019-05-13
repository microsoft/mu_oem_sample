/**@file Library to interface with alternate boot variable

Copyright (c) 2018, Microsoft Corporation

All rights reserved.
Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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
