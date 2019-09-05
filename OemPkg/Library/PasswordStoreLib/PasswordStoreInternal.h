/** @file -- PasswordStoreLib.h

  Interfaces to the password store.

  Copyright (C) Microsoft Corporation. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef __PASSWORD_STORE_INTERNAL_H__
#define __PASSWORD_STORE_INTERNAL_H__


/**
  Performs any initialization that is necessary for the functions in this
  library to behave as expected.

  Only necessary to run once per boot.
  Published as a public function so that it can be invoked in a useful driver.

  @retval     EFI_SUCCESS   Initialization is complete.
  @retval     Others        Something went wrong. Investigate further.

**/
EFI_STATUS
EFIAPI
PasswordStoreInitializeLib (
  VOID
  );


/**
  Deletes all passwords and resets password infrastructure to factory condition.
  Published as a public function so that it can be invoked in a useful driver.

  @retval     EFI_SUCCESS   Reset is complete.
  @retval     Others        Something went wrong. Investigate further.

**/
EFI_STATUS
EFIAPI
PasswordStoreResetPasswordLib (
  VOID
  );


#endif // _PASSWORD_STORE_INTERNAL_H_
