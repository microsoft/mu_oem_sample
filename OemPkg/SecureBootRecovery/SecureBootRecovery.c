/** @file
  This application will attempt to append the 'Windows UEFI CA 2023' and then reboot the system.
  On success, this application will allow the system to boot into 2023 signed Windows

  Copyright (C) Microsoft Corporation
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Uefi.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Guid/ImageAuthentication.h>

#include "RecoveryPayload.h"

// 10 seconds in microseconds
#define STALL_10_SECONDS  10000000

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
  UINT32      Attributes;

  Attributes = EFI_VARIABLE_NON_VOLATILE |
               EFI_VARIABLE_BOOTSERVICE_ACCESS |
               EFI_VARIABLE_RUNTIME_ACCESS |
               EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS |
               EFI_VARIABLE_APPEND_WRITE;

  //
  // Start checking that the system is in a state we can safely use
  //
  if (SystemTable == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if ((SystemTable->ConOut == NULL) || (SystemTable->ConOut->OutputString == NULL) || (SystemTable->ConOut->ClearScreen == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  if ((SystemTable->BootServices == NULL) || (SystemTable->BootServices->Stall == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // After this point, we should be able to print and stall but nothing else has been verified
  //
  if ((SystemTable->RuntimeServices == NULL) || (SystemTable->RuntimeServices->ResetSystem == NULL)) {
    Status = EFI_INVALID_PARAMETER;
    goto Exit;
  }

  //
  // Start informing the user of what is happening
  //
  Status = SystemTable->ConOut->ClearScreen (SystemTable->ConOut);
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  Status = SystemTable->ConOut->OutputString (SystemTable->ConOut, L"\r\nAttempting to update the system's secureboot certificates\r\n");
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  SystemTable->ConOut->OutputString (SystemTable->ConOut, L"Learn more about this tool at https://aka.ms/securebootrecovery\r\n");

  //
  // Perform the append operation
  //
  Status = SystemTable->RuntimeServices->SetVariable (
                                           L"db",
                                           &gEfiImageSecurityDatabaseGuid,
                                           Attributes,
                                           sizeof (mDbUpdate),
                                           mDbUpdate
                                           );
  if (EFI_ERROR (Status)) {
    //
    // On failure, inform the user and reboot
    // Likely this will continue to fail on reboot, the user will hopefully go to https://aka.ms/securebootrecovery to learn more
    //
    SystemTable->ConOut->OutputString (SystemTable->ConOut, L"\r\nFailed to update the system's secureboot keys\r\n");
    goto Reboot;
  }

  //
  // Otherwise the system took the update, so let's inform the user
  //
  SystemTable->ConOut->OutputString (SystemTable->ConOut, L"\r\nSuccessfully updated the system's secureboot keys\r\n");

Reboot:

  //
  //  Stall for 10 seconds to give the user a chance to read the error message
  //
  SystemTable->BootServices->Stall (STALL_10_SECONDS);

  //
  // Reset the system
  //
  SystemTable->RuntimeServices->ResetSystem (EfiResetCold, EFI_SUCCESS, 0, NULL);

Exit:
  //
  // If we get here, something really bad happened and we don't have a means to recover
  //

  //
  // let's atleast try to print an error to the console
  //
  SystemTable->ConOut->OutputString (SystemTable->ConOut, L"Exiting unexpectedly!\r\n");

  //
  // Stall for 10 seconds to give the user a chance to read the error message
  //
  SystemTable->BootServices->Stall (STALL_10_SECONDS);

  return Status;
}
