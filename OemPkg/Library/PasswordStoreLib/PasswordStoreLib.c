/** @file PasswordStore.c

  Copyright (C) Microsoft Corporation. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent

  Manages storage location for the platform ADMIN Password.

  The UEFI System Password set/delete interface

**/

#include <PiDxe.h>

// Platform defined variable to hold the password hash

#include <Guid/PasswordStoreVariable.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PasswordPolicyLib.h>
#include <Library/PasswordStoreLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiLib.h>

#include "PasswordStoreInternal.h"

#define PASSWORD_STORE_LIB_INSTALLED_GUID \
  { /* 68e6a3c8-2138-4846-a085-73eab4c344e0 */ \
    0x68e6a3c8, 0x2138, 0x4846, {0xa0, 0x85, 0x73, 0xea, 0xb4, 0xc3, 0x44, 0xe0 } \
  }

CONST CHAR16    *mPasswordName = {
  PASSWORD_STORE_ADMIN_VARIABLE_NAME
};

STATIC       EFI_GUID       mPasswordStoreInstalledGuid = PASSWORD_STORE_LIB_INSTALLED_GUID;
STATIC       PASSWORD_HASH  mNullPasswordHash;
STATIC       UINTN          mNullPasswordHashSize;


/**
  Set the password variable.

  @param[in]  PasswordHash        Pointer to the password hash
  @param[in]  PasswordHashSize    Size of the password hash

  @retval     EFI_SUCCESS   Password stored successfully.
  @retval     Others        Something went wrong. Investigate further.
**/
EFI_STATUS
EFIAPI
PasswordStoreSetPassword (
  IN  CONST UINT8     *PasswordHashValue,
  IN        UINTN      PasswordHashSize
)
{
  EFI_STATUS     Status;
  PASSWORD_HASH  PasswordHash;

  if (PasswordHashValue == NULL)
  {
    return EFI_INVALID_PARAMETER;
  }

  PasswordHash = (PASSWORD_HASH) PasswordHashValue;
  Status = PasswordPolicyValidatePasswordHash (PasswordHash, PasswordHashSize);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  Status = gRT->SetVariable(PASSWORD_STORE_ADMIN_VARIABLE_NAME,
    &PASSWORD_STORE_ADMIN_NAMESPACE_GUID,
    PASSWORD_STORE_ADMIN_VARIABLE_ATTRS,
    PasswordHashSize,
    (VOID *) PasswordHash);
  if (EFI_ERROR(Status))
  {
    Status = EFI_ABORTED;
  }
  return Status;
}


/**
  Public interface for determining whether a given password is set.

  NOTE: Will initialize the Password Store if it doesn't exist

  @retval     TRUE    Password is set.
  @retval     FALSE   Password is not set, or an error occurred preventing
                      the check from completing successfully.

**/
BOOLEAN
EFIAPI
PasswordStoreIsPasswordSet (
  )
{
  EFI_STATUS          Status;
  BOOLEAN             Result = FALSE;
  UINTN               BufferSize;
  PASSWORD_HASH       Store = NULL;

  // Attempt to retrieve the password.
  Status = GetVariable2( PASSWORD_STORE_ADMIN_VARIABLE_NAME,
                            &PASSWORD_STORE_ADMIN_NAMESPACE_GUID,
                         (VOID **) &Store,
                        &BufferSize);

  // If it's missing entirely, we need to set it.
  if (Status == EFI_NOT_FOUND || BufferSize == 0)
  {
    Status = PasswordStoreSetPassword(mNullPasswordHash, mNullPasswordHashSize);
  }
  // Determine whether the current password is valid.
  else if (!EFI_ERROR( Status ) && BufferSize > 0)
  {
    // Determine whether the current password is the NULL password.
    if (BufferSize != mNullPasswordHashSize &&
        CompareMem( Store, mNullPasswordHash, mNullPasswordHashSize ) != 0)
    {
      Result = TRUE;
    }
  }

  if (NULL != Store)
  {
    FreePool (Store);
  }

  return Result;
} // IsPasswordSet()


/**
  Public interface for validating a password against the current password.

  If no password is currently set, will return FALSE.

  NOTE: This function does NOT perform string validation on the password
        being authenticated. This is to accommodate changing valid character sets.
        Will still make sure that string does not exceed max buffer size.

  @param[in]  Password  String being evaluated.

  @retval     TRUE      Password matches the stored password for Handle.
  @retval     TRUE      No password is currently set.
  @retval     TRUE      Password is NULL and Handle has previously authenticated successfully.
  @retval     FALSE     No Password is set for Handle.
  @retval     FALSE     Supplied Password does not match stored password for Handle.

**/
BOOLEAN
EFIAPI
PasswordStoreAuthenticatePassword (
  IN  CONST CHAR16   *Password
  )
{
  EFI_STATUS          Status = EFI_SUCCESS;
  BOOLEAN             Result = FALSE;
  UINTN               DataSize;
  CHAR16              TempPassword[PW_MAX_LENGTH + 1];  // Maximum password length plus a NULL terminator.
  PASSWORD_HASH       CurStore = NULL;
  PASSWORD_HASH       NewStore = NULL;

  // If there is no password set, all accesses should authenticate.
  if (!PasswordStoreIsPasswordSet())
  {
    return TRUE;
  }

  // If no password is supplied, return the cached Authentication state.
  // NOTE: This cache exists so that calling functions don't have to hold on to
  //       a copy of the password buffer to authenticate future actions (and so
  //       that the user doesn't have to enter their password half a dozen times).
  //       This should be replaced with a more robust "Session token" if this system
  //       is leveraged for anything more complicated than FrontPage admin password.
  if (Password == NULL || Password[0] == '\0')
  {
     return FALSE; // if or not a null password is set is checked in IsPasswordSet
  }

  // Prep the password for evaluation.
  PasswordPolicySafeCopyPassword(TempPassword,
    ARRAY_SIZE(TempPassword),
    Password);

  //
  // Step 1: Retrieve the current store.
  Status = GetVariable2( PASSWORD_STORE_ADMIN_VARIABLE_NAME,
                        &PASSWORD_STORE_ADMIN_NAMESPACE_GUID,
                         (VOID **) &CurStore,
                        &DataSize);
  // NOTE: DataSize should now reflect the size of the variable when it was saved.

  //
  // Step 2: If the current store was retrieved, build a new store to compare it to.
  if (!EFI_ERROR( Status ))
  {
    // Build out the rest of the store so that we can compare the keys.
    Status = PasswordPolicyGeneratePasswordHash( CurStore,      // Use existing Version and Salt
                                                 TempPassword,  // Password
                                                &NewStore,      // Store
                                                &DataSize);
  }

  // Always put away your toys.
  PasswordPolicyCleansePwBuffer(TempPassword, sizeof( TempPassword ));

  //
  // Step 3: Compare the store.
  // NOTE: Sure, it would be faster to just compare the key, but I don't think this hurts anything.
  if (!EFI_ERROR( Status ))
  {
    Result = (CompareMem( CurStore, NewStore, DataSize ) == 0);
  }

  if (NULL != CurStore)
  {
    FreePool (CurStore);
  }
  if (NULL != NewStore)
  {
    FreePool (NewStore);
  }

  return Result;
} // AuthenticatePassword()

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
  )
{
  EFI_STATUS    Status;
  CHAR16        *PasswordName;

  // Attempt to delete the password.
  PasswordName  = (CHAR16*)mPasswordName;
  Status        = gRT->SetVariable( PasswordName,
                                    &PASSWORD_STORE_ADMIN_NAMESPACE_GUID,
                                    0,
                                    0,
                                    NULL );
  if (Status == EFI_NOT_FOUND)
  {
    Status = EFI_SUCCESS;
  }
  if (EFI_ERROR( Status ))
  {
    DEBUG(( DEBUG_ERROR, "%a - Failed to properly reset password! Status = %r.\n", __FUNCTION__, Status ));
    ASSERT( FALSE );
  }

  return Status;
} // ResetPasswordLib()


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
  )
{
  EFI_STATUS                    Status;
  CHAR16                        *PasswordName;
  UINT32                        Attributes;
  UINTN                         DataSize;
  PASSWORD_HASH                 Store;

  PasswordName  = (CHAR16*)mPasswordName;
  //1. Load Variable
  Status = GetVariable3( PasswordName,
                        &PASSWORD_STORE_ADMIN_NAMESPACE_GUID,
                         (VOID **) &Store,
                        &DataSize,
                        &Attributes);
  if (!EFI_ERROR(Status))
  {
    FreePool (Store);
  }

  // Make sure that password is found and has the correct attributes.
  if (Status == EFI_NOT_FOUND || Attributes != PASSWORD_STORE_ADMIN_VARIABLE_ATTRS)
  {
    // If invalid, initialize it.
    Status = PasswordStoreSetPassword (mNullPasswordHash, mNullPasswordHashSize);
  }
  if (EFI_ERROR( Status ))
  {
    DEBUG(( DEBUG_ERROR, "%a - Failed to properly initialize password! Status = %r.\n", __FUNCTION__, Status ));
    ASSERT( FALSE );
  }

  return Status;
} // PasswordStoreInitializePasswordLib()


/**
The constructor function initializes the Lib for Dxe.

This constructor is only needed for MsSettingsManager support.
The design is to have the PCD false fall all modules except the 1 that should support the MsSettingsManager.  Because this
is a build time PCD

The constructor function publishes Performance and PerformanceEx protocol, allocates memory to log DXE performance
and merges PEI performance data to DXE performance log.
It will ASSERT() if one of these operations fails and it will always return EFI_SUCCESS.

@param  ImageHandle   The firmware allocated handle for the EFI image.
@param  SystemTable   A pointer to the EFI System Table.

@retval EFI_SUCCESS   The constructor always returns EFI_SUCCESS.

**/
EFI_STATUS
EFIAPI
PasswordStoreLibConstructor
(
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{

  EFI_STATUS                    Status;
  VOID                         *Dummy;

  Status = PasswordPolicyGeneratePasswordHash (NULL, NULL, &mNullPasswordHash, &mNullPasswordHashSize);

  Status = gBS->LocateProtocol(&mPasswordStoreInstalledGuid,
                               NULL,
                               (VOID **)&Dummy);

  if (Status == EFI_NOT_FOUND)
  {
    // Only initialize the library once regardless of the number of instances of this
    // library

    Status = gBS->InstallProtocolInterface (&gImageHandle,
                                            &mPasswordStoreInstalledGuid,
                                             EFI_NATIVE_INTERFACE,
                                             NULL);
    if (EFI_ERROR(Status))
    {
      DEBUG(( DEBUG_ERROR, "%a: - Unable to install already-installed protocol- %r\n", __FUNCTION__, Status ));
    }

    PasswordStoreInitializeLib ();

    // There is an assumption that the Password Variable will be locked.  That will
    // be up to the platform.
  }

  return EFI_SUCCESS;
}
