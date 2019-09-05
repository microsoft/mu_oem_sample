/** @file PasswordStoreVariable.h

  This file defines the GUID and variable names for a variable backed PasswordStore

  Copyright (C) Microsoft Corporation. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef __PASSWORD_STORE_VARIABLE_GUID_H__
#define __PASSWORD_STORE_VARIABLE_GUID_H__


#define PASSWORD_STORE_ADMIN_NAMESPACE_GUID    gOemPkgPasswordStoreVarGuid
#define PASSWORD_STORE_ADMIN_VARIABLE_NAME     L"Passw0rd"
#define PASSWORD_STORE_ADMIN_VARIABLE_ATTRS    (EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS)   // Non-volatile, BS-only.


extern EFI_GUID gOemPkgPasswordStoreVarGuid;

#endif

