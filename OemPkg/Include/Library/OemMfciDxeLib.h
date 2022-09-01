/** @file

  Helper library to support the registration of MFCI policy change notification
  during DXE phase.

  Copyright (c) Microsoft Corporation
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef OEM_MFCI_DXE_LIB_H_
#define OEM_MFCI_DXE_LIB_H_

#include <MfciPolicyType.h>
#include <Protocol/MfciProtocol.h>
#include <Library/OemMfciLib.h>

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
  );

#endif //OEM_MFCI_DXE_LIB_H_
