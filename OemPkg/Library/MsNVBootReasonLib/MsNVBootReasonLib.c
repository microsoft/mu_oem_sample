/*++

Library to provide interface on Reboot Reason non volatile varialbles

Copyright (c) 2015 - 2018, Microsoft Corporation

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

#include <Uefi.h>                                     // UEFI base types

#include <Guid/GlobalVariable.h>

#include <Library/UefiRuntimeServicesTableLib.h>      // gRT
#include <Library/DebugLib.h>                         // DEBUG tracing
#include <Library/DeviceBootManagerLib.h>
#include <Library/MsNVBootReasonLib.h>

#include "MsNVBootReasonHdr.h"

/**
  Update secure boot violation

  @param[in]        RebootStatus  Reboot Status from BDS

  @retval  EFI_SUCCESS  Update secure boot violation successfully
  @retval  !EFI_SUCCESS Failed to update secure boot violation
**/
EFI_STATUS
UpdateSecureBootViolation (
  IN  EFI_STATUS    RebootStatus
) 
{
  CHAR16     *SbViolationVarName = SFP_SB_VIOLATION_SIGNAL_VAR_NAME;
  BOOLEAN     SecViolation;
  EFI_STATUS  Status;

  if (EFI_SECURITY_VIOLATION == RebootStatus) {
      SecViolation = TRUE;
      Status = gRT->SetVariable(SbViolationVarName,
                                &gOemBootNVVarGuid,
                                EFI_VARIABLE_BOOTSERVICE_ACCESS,    // This variable is volatile.
                                sizeof( SecViolation ),
                                (UINT8*) &SecViolation );
      DEBUG((DEBUG_INFO,"Detected SecureBootFail (2)\n"));
  } else {
      Status = EFI_SUCCESS;
  }
  return Status;
}

/**
  Set the Reboot Reason

  @param[in]        RebootStatus  Reboot Status from BDS

  @retval  EFI_SUCCESS  Set reboot reason successfully
  @retval  !EFI_SUCCESS Failed to set reboot reason
**/
EFI_STATUS
SetRebootReason (
  IN  EFI_STATUS     RebootStatus
) 
{
  CHAR8      *RebootReason;
  EFI_STATUS  Status;

  if (EFI_SECURITY_VIOLATION == RebootStatus) {
    Status = UpdateSecureBootViolation (RebootStatus);
    RebootReason = MSP_REBOOT_REASON_SETUP_SEC_FAIL;
  }  else if (OEM_REBOOT_TO_SETUP_KEY == RebootStatus) {
    RebootReason = MSP_REBOOT_REASON_SETUP_KEY;
  }  else if (OEM_REBOOT_TO_SETUP_OS == RebootStatus) {
    RebootReason = MSP_REBOOT_REASON_SETUP_OS;
  }  else if (EFI_ERROR(RebootStatus)) {
    RebootReason = MSP_REBOOT_REASON_SETUP_BOOTFAIL;
  }  else {
    RebootReason = MSP_REBOOT_REASON_SETUP_NONE;
  }

  Status = gRT->SetVariable(
      MSP_REBOOT_REASON_VAR_NAME,
      &gOemBootNVVarGuid,
      EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_NON_VOLATILE,
      MSP_REBOOT_REASON_LENGTH,
      RebootReason
      );
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "%a unable to update RebootReason. Code=%r\n", __FUNCTION__, Status));
  }
  DEBUG((DEBUG_INFO, "%a new reboot reason is %a. Code=%r\n", __FUNCTION__, RebootReason, Status));

  return Status;
}

/**
  Remove reboot reason

  @retval  EFI_SUCCESS  Cleaned Reboot reason successfully
  @retval  !EFI_SUCCESS Failed to clean Reboot reason
**/
EFI_STATUS
EFIAPI
ClearRebootReason(
  VOID
)
{
  EFI_STATUS Status;
  
  Status = gRT->SetVariable (
                MSP_REBOOT_REASON_VAR_NAME,
                &gOemBootNVVarGuid,
                EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_NON_VOLATILE,
                0,
                NULL
                );
  return Status;
}

/**
  Read reboot reason

  @param[out]       Buffer        Buffer to hold returned data
  @param[in, out]   BufferSize    Input as available data buffer size, output as data 
                                  size filled

  @retval  EFI_SUCCESS  Fetched version information successfully
  @retval  !EFI_SUCCESS Failed to fetch version information
**/
EFI_STATUS
EFIAPI
GetRebootReason(
      OUT UINT8                  *Buffer,          OPTIONAL
  IN  OUT UINTN                  *BufferSize
)
{
  EFI_STATUS Status;

  if ((BufferSize == NULL) || 
      ((*BufferSize != 0) && (Buffer == NULL))) {
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }
  
  if (*BufferSize < MSP_REBOOT_REASON_LENGTH) {
    *BufferSize = MSP_REBOOT_REASON_LENGTH;
    Status = EFI_BUFFER_TOO_SMALL;
    goto Done;
  }

  *BufferSize = MSP_REBOOT_REASON_LENGTH;
  Status = gRT->GetVariable (
              MSP_REBOOT_REASON_VAR_NAME,
              &gOemBootNVVarGuid,
              NULL,
              BufferSize,
              Buffer
              );

Done:
  return Status;
}

/**
  Get the current Reboot Reason and update based on OS entry to FrontPage

  @retval  EFI_SUCCESS  Updated reboot reason successfully
  @retval  !EFI_SUCCESS Failed to update reboot reason
**/
EFI_STATUS
EFIAPI
UpdateRebootReason (
  VOID
)
{
  UINTN       DataSize;
  UINT64      OsIndication;
  CHAR8       RebootReason[MSP_REBOOT_REASON_LENGTH + 1] = MSP_REBOOT_REASON_SETUP_NONE;
  EFI_STATUS  Status;


  DataSize = MSP_REBOOT_REASON_LENGTH;
  Status = gRT->GetVariable (
              MSP_REBOOT_REASON_VAR_NAME,
              &gOemBootNVVarGuid,
              NULL,
              &DataSize,
              RebootReason
              );
  if (EFI_ERROR(Status) && (EFI_NOT_FOUND != Status)) {
    DEBUG((DEBUG_ERROR, "%a error reading RebootReason. Code = %r\n", __FUNCTION__, Status));
  } else {
    if ((RebootReason[0] == 'B') && (RebootReason[1] == 'S')) {
      Status = UpdateSecureBootViolation(EFI_SECURITY_VIOLATION);
    }

    // Check for OS Indications to set FrontPage icon if there is no other reboot reason
    //
    if (RebootReason[0] != 'B') {
        OsIndication = 0;
        DataSize = sizeof(UINT64);
        Status = gRT->GetVariable (
                        EFI_OS_INDICATIONS_VARIABLE_NAME,
                        &gEfiGlobalVariableGuid,
                        NULL,
                        &DataSize,
                        &OsIndication
                        );

        if ((DataSize == sizeof (OsIndication)) &&
            ((OsIndication & EFI_OS_INDICATIONS_BOOT_TO_FW_UI) != 0)) {
            Status = SetRebootReason (OEM_REBOOT_TO_SETUP_OS);
        }
    }
  }

  DEBUG((DEBUG_INFO, "%a current reboot reason is %a. Code=%r\n", __FUNCTION__, RebootReason, Status));
  return Status;
}
