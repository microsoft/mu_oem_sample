/** @file
  This sample application that is the simplest UEFI application possible.
  It simply prints "Hello Uefi!" to the UEFI Console Out device and stalls the CPU for 30 seconds.

  Copyright (C) Microsoft Corporation
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Uefi.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Guid/ImageAuthentication.h>

#include "RecoveryPayload.h"

// 30 seconds in microseconds
#define STALL_30_SECONDS  10000000

/**
  The user Entry Point for Application. The user code starts with this function
  as the real entry point for the application.

  @param[in] ImageHandle        The firmware allocated handle for the EFI image.
  @param[in] SystemTable        A pointer to the EFI System Table.

  @retval EFI_SUCCESS           The entry point is executed successfully.
  @retval EFI_INVALID_PARAMETER SystemTable provided was not valid.
  @retval other                 Some error occurs when executing this entry point.

**/
EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS  Status;

  UINT32  Attributes = EFI_VARIABLE_NON_VOLATILE |
                       EFI_VARIABLE_BOOTSERVICE_ACCESS |
                       EFI_VARIABLE_RUNTIME_ACCESS |
                       EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS |
                       EFI_VARIABLE_APPEND_WRITE;

  if (SystemTable == NULL) {
    Status = EFI_INVALID_PARAMETER;
    goto Exit;
  }

  if ((SystemTable->ConOut == NULL) || (SystemTable->ConOut->OutputString == NULL) || (SystemTable->ConOut->ClearScreen == NULL)) {
    Status = EFI_INVALID_PARAMETER;
    goto Exit;
  }

  if ((SystemTable->BootServices == NULL) || (SystemTable->BootServices->Stall == NULL)) {
    Status = EFI_INVALID_PARAMETER;
    goto Exit;
  }

  if ((SystemTable->RuntimeServices == NULL) || (SystemTable->RuntimeServices->ResetSystem == NULL)) {
    Status = EFI_INVALID_PARAMETER;
    goto Exit;
  }

  Status = SystemTable->ConOut->ClearScreen (SystemTable->ConOut);
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  Status = SystemTable->ConOut->OutputString (SystemTable->ConOut, L"\r\nAttempting to update the system's secureboot keys\r\n");
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  SystemTable->ConOut->OutputString (SystemTable->ConOut, L"Learn more about this tool at https://aka.ms/securebootrecovery\r\n");

  Status = SystemTable->RuntimeServices->SetVariable (
    L"db",
    &gEfiImageSecurityDatabaseGuid,
    EFI_VARIABLE_NON_VOLATILE |
    EFI_VARIABLE_BOOTSERVICE_ACCESS | 
    EFI_VARIABLE_RUNTIME_ACCESS | 
    EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS |
    EFI_VARIABLE_APPEND_WRITE,
    sizeof (mDbUpdate),
    mDbUpdate
  );

  if (EFI_ERROR (Status)) {
    SystemTable->ConOut->OutputString (SystemTable->ConOut, L"\r\nFailed to update the system's secureboot keys\r\n");
    goto Reboot;
  }

  SystemTable->ConOut->OutputString (SystemTable->ConOut, L"\r\nSuccessfully updated the system's secureboot keys\r\n");

Reboot:
  SystemTable->BootServices->Stall (STALL_30_SECONDS);

  // Reboot
  SystemTable->RuntimeServices->ResetSystem (EfiResetCold, EFI_SUCCESS, 0, NULL);

Exit:
  SystemTable->ConOut->OutputString (SystemTable->ConOut, L"Exiting unexpectedly..\r\n");

  SystemTable->BootServices->Stall (STALL_30_SECONDS);

  // Should never get here
  return Status;
}
