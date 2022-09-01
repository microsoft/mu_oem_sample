/* @file OemMfciLibPei.c

  MFCI based library instance for system mode related functions for
  configuration modules on QEMU Q35 platform.

  Copyright (c) Microsoft Corporation.
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/
#include <PiPei.h>
#include <OemMfciDefines.h>
#include <Ppi/MfciPolicyPpi.h>

#include <Library/DebugLib.h>
#include <Library/PeiServicesLib.h>
#include <Library/OemMfciLib.h>

/**
 * Inspect whether the current operation mode is categorized as manufacturing mode.
 *
 * @param[out] OperationMode      64 bit value from MFCI framework, indicating the current operation mode
 *
 * @return OEM_UEFI_CUSTOMER_MODE         Current mode is customer mode.
 * @return OEM_UEFI_MANUFACTURING_MODE    Current operation mode is manufacturing mode.
 */
OEM_UEFI_OPERATION_MODE
EFIAPI
GetMfciSystemOperationMode (
  VOID
  )
{
  EFI_STATUS               Status;
  MFCI_POLICY_PPI          *MfciPpi;
  MFCI_POLICY_TYPE         CurrentMfciPolicy;
  OEM_UEFI_OPERATION_MODE  Result;

  // First, locate the required PPI.
  Status = PeiServicesLocatePpi (&gMfciPpiGuid, 0, NULL, (VOID **)&MfciPpi);
  if (EFI_ERROR (Status)) {
    // If we can't locate the PPI we need to default to the CUSTOMER mode to be on the safe side.
    DEBUG ((DEBUG_ERROR, "%a Failed to locate MFCI PPI - %r!!!\n", __FUNCTION__, Status));
    Result = OEM_UEFI_CUSTOMER_MODE;
    goto Exit;
  }

  // Check the OEM Manufacturing mode state bit in the 64 bit MFCI Policy.
  CurrentMfciPolicy = MfciPpi->GetMfciPolicy (MfciPpi);
  if (CurrentMfciPolicy & OEM_MFCI_STATE_MFG_MODE) {
    // UEFI is in Manufacturing mode
    Result = OEM_UEFI_MANUFACTURING_MODE;
    goto Exit;
  } else {
    // UEFI is in Customer mode.
    Result = OEM_UEFI_CUSTOMER_MODE;
    goto Exit;
  }

Exit:
  return Result;
}
