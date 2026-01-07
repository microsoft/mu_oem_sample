/** @file DeviceStateDxe.c

  This platform module sets the DEVICE_STATE bits prior to display.

  This driver currently implements the following bits in a standard method:
    DEVICE_STATE_SECUREBOOT_OFF

  This library can either mask those bits (force low) or add other bits.  See the
  MdeModulePkg/Include/Library/DeviceStateLib.h file for bit definitions.

  Copyright (C) Microsoft Corporation.

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#include <PiDxe.h>
#include <Library/DebugLib.h>
#include <Library/DeviceStateLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include <Guid/GlobalVariable.h>

/**
  Helper function to query whether the secure boot variable is in place.
  For Project Mu Code if the PK is set then Secure Boot is enforced (there is no
  SetupMode)

  @retval     TRUE if secure boot is enabled, FALSE otherwise.
**/
BOOLEAN
IsSecureBootOn (
  )
{
  EFI_STATUS  Status;
  UINTN       PkSize = 0;

  Status = gRT->GetVariable (EFI_PLATFORM_KEY_NAME, &gEfiGlobalVariableGuid, NULL, &PkSize, NULL);
  if ((Status == EFI_BUFFER_TOO_SMALL) && (PkSize > 0)) {
    DEBUG ((DEBUG_INFO, "%a - PK exists.  Secure boot on.  Pk Size is 0x%X\n", __FUNCTION__, PkSize));
    return TRUE;
  }

  DEBUG ((DEBUG_INFO, "%a - PK doesn't exist.  Secure boot off\n", __FUNCTION__));
  return FALSE;
}

/**
  Module Entrypoint.
  Check States and Set State

  @param[in]  FileHandle    Handle of the file being invoked.
  @param[in]  SystemTable   Pointer to the system table.

  @retval     EFI_SUCCESS  Always returns success.
**/
EFI_STATUS
EFIAPI
DeviceStateDxeEntry (
  IN EFI_HANDLE        FileHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  DEVICE_STATE  State;

  State = 0;

  if (!IsSecureBootOn ()) {
    State |= DEVICE_STATE_SECUREBOOT_OFF;
  }

  AddDeviceState (State);

  return EFI_SUCCESS;
}
