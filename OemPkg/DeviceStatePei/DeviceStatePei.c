/** @file DeviceStatePei.c

  This platform module sets the DEVICE_STATE bits prior to display.

  This driver currently implements the following bits in a standard method:
    DEVICE_STATE_SECUREBOOT_OFF

  This library can either mask those bits (force low) or add other bits.  See the
  MdeModulePkg/Include/Library/DeviceStateLib.h file for bit definitions.

  Copyright (C) Microsoft Corporation.

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#include <PiPei.h>
#include <Library/DebugLib.h>
#include <Library/DeviceStateLib.h>
#include <Library/PeiServicesLib.h>

#include <Ppi/ReadOnlyVariable2.h>

#include <Guid/GlobalVariable.h>

/**
  Helper function to query whether the secure boot variable is in place.
  For Project Mu Code if the PK is set then Secure Boot is enforced (there is no
  SetupMode)

  @retval     TRUE if secure boot is enabled, FALSE otherwise.
**/
BOOLEAN
IsSecureBootOn()
{
  EFI_STATUS Status;
  EFI_PEI_READ_ONLY_VARIABLE2_PPI *VarPpi = NULL;
  UINTN PkSize = 0;

  Status = PeiServicesLocatePpi(&gEfiPeiReadOnlyVariable2PpiGuid, 0, NULL, (VOID *)&VarPpi);
  if (EFI_ERROR(Status)){
    DEBUG((DEBUG_ERROR, "Failed to locate EFI_PEI_READ_ONLY_VARIABLE2_PPI. \n"));
    return FALSE;
  }

  Status = VarPpi->GetVariable(VarPpi, EFI_PLATFORM_KEY_NAME, &gEfiGlobalVariableGuid, NULL, &PkSize, NULL);
  if ((Status == EFI_BUFFER_TOO_SMALL) && (PkSize > 0) )
  {
    DEBUG((DEBUG_INFO, "%a - PK exists.  Secure boot on.  Pk Size is 0x%X\n", __FUNCTION__, PkSize));
    return TRUE;
  }
  DEBUG((DEBUG_INFO, "%a - PK doesn't exist.  Secure boot off\n", __FUNCTION__));
  return FALSE;
}

/**
  Module Entrypoint.
  Check States and Set State

  @param[in]  FileHandle   Handle of the file being invoked.

  @param[in]  PeiServices  General purpose services available to every PEIM.

  @retval     EFI_SUCCESS  Always returns success.
**/
EFI_STATUS
EFIAPI
DeviceStatePeiEntry(
  IN       EFI_PEI_FILE_HANDLE       FileHandle,
  IN CONST EFI_PEI_SERVICES          **PeiServices
  )
{
  DEVICE_STATE                  State;
  State = 0;

  if (!IsSecureBootOn())
  {
    State |= DEVICE_STATE_SECUREBOOT_OFF;
  }

  AddDeviceState(State);

  return EFI_SUCCESS;
}