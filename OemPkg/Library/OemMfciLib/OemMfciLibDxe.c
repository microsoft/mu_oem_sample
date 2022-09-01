/* @file OemMfciLibDxe.c

  MFCI based library instance for system mode related functions for
  configuration modules on QEMU Q35 platform.

  Copyright (c) Microsoft Corporation.
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/
#include <PiDxe.h>
#include <MfciPolicyType.h>
#include <OemMfciDefines.h>
#include <Protocol/MfciProtocol.h>

#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/OemMfciLib.h>

MFCI_PROTOCOL  *mMfciPolicyProtocol = NULL;

/**
  Library function to register a callback when the operation mode encounters
  a transition.

  The registered callbacks will be dispatched at gMsStartOfBdsNotifyGuid event
  prior to EndOfDxe.

  @param[in] Callback           Pointer to the callback function being registered.

  @retval EFI_SUCCESS           Callback was successfully registered.
  @retval EFI_ALREADY_STARTED   We have passed EndOfDxe and this callback no longer
                                makes sense.
  @retval Others                Callback registration failed.

**/
EFI_STATUS
EFIAPI
RegisterMfciOperationModeChangeNotifyCallback (
  IN MFCI_POLICY_CHANGE_CALLBACK  Callback
  )
{
  EFI_STATUS  Status;

  // First, locate the required protocol, if not already available
  if (mMfciPolicyProtocol == NULL) {
    Status = gBS->LocateProtocol (&gMfciProtocolGuid, NULL, (VOID **)&mMfciPolicyProtocol);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "%a Failed to locate MFCI Protocol - %r!!!\n", __FUNCTION__, Status));
      goto Exit;
    }
  }

  // Then we can get to the real work...
  Status = mMfciPolicyProtocol->RegisterMfciPolicyChangeCallback (mMfciPolicyProtocol, Callback);
  if (EFI_ERROR (Status)) {
    // Failed to register this callback, propagate the error code to caller.
    DEBUG ((DEBUG_ERROR, "%a Failed to register MFCI change callback - %r!!!\n", __FUNCTION__, Status));
    goto Exit;
  }

Exit:
  return Status;
}

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
  MFCI_POLICY_TYPE         CurrentMfciPolicy;
  OEM_UEFI_OPERATION_MODE  Result;

  // First, locate the required protocol.
  if (mMfciPolicyProtocol == NULL) {
    Status = gBS->LocateProtocol (&gMfciProtocolGuid, NULL, (VOID **)&mMfciPolicyProtocol);
    if (EFI_ERROR (Status)) {
      // If we can't locate the protocol we need to default to the CUSTOMER mode to be on the safe side.
      DEBUG ((DEBUG_ERROR, "%a Failed to locate MFCI Protocol - %r!!!\n", __FUNCTION__, Status));
      Result = OEM_UEFI_CUSTOMER_MODE;
      goto Exit;
    }
  }

  // Check the OEM Manufacturing mode state bit in the 64 bit MFCI Policy.
  CurrentMfciPolicy = mMfciPolicyProtocol->GetMfciPolicy (mMfciPolicyProtocol);
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
