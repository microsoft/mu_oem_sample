/** @file
  This sample application that is the simplest UEFI application possible.
  It simply prints "Hello Uefi!" to the UEFI Console Out device and stalls the CPU for 30 seconds.

  Copyright (C) Microsoft Corporation
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Uefi.h>
#include <Library/UefiApplicationEntryPoint.h>

// 30 seconds in microseconds
#define STALL_30_SECONDS  30000000

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

  if (SystemTable == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if ((SystemTable->ConOut == NULL) || (SystemTable->ConOut->OutputString == NULL) || (SystemTable->ConOut->ClearScreen == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  if ((SystemTable->BootServices == NULL) || (SystemTable->BootServices->Stall == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  Status = SystemTable->ConOut->ClearScreen (SystemTable->ConOut);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = SystemTable->ConOut->OutputString (SystemTable->ConOut, L"\r\nHello Uefi!\r\n");
  if (EFI_ERROR (Status)) {
    return Status;
  }

  SystemTable->BootServices->Stall (STALL_30_SECONDS);

  return EFI_SUCCESS;
}
