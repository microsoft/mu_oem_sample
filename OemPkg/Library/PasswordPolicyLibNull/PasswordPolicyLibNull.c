/** @file PasswordPolicyLib.c

  Library to support password hashes.

  Copyright (C) Microsoft Corporation. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <PiDxe.h>

#include <Library/PasswordPolicyLib.h>

/**
  Copies a password to a buffer, but will only copy the maximum
  characters. Always places a NULL terminator at the end of the
  output string.

  NOTE: Buffer MUST be at least PW_MAX_LENGTH + 1 in length.

  @param[out] Buffer        Pointer to an output buffer for the final password.
                            MUST be at least PW_MAX_LENGTH + 1 of CHAR16 in size.
  @param[in]  BufferLength  Length of the output buffer (in CHAR16s).
  @param[in]  Password      Pointer to the password to be copied.

  @retval     EFI_SUCCESS           String copied.
  @retval     EFI_BUFFER_TOO_SMALL  Buffer cannot hold a max-length password.

**/
EFI_STATUS
EFIAPI
PasswordPolicySafeCopyPassword (
  OUT CHAR16        *Buffer,
  IN  UINTN          BufferLength,
  IN  CONST CHAR16  *Password
  )
{

  return EFI_UNSUPPORTED;
} // PasswordPolicySafeCopyPassword()

/**
  Takes in a buffer and destroys the data within it.

  @param[in]  Data  Pointer to the buffer in question.
  @param[in]  Size  Size of Data.

**/
VOID
EFIAPI
PasswordPolicyCleansePwBuffer (
  IN  CHAR16  *Data,
  IN  UINTN    Size
  )
{

  return;
} // PasswordPolicyCleansePwBuffer()

/**
  Public interface for validating password strings.

  Will evaluate all current password strength/validity requirements and
  return a BOOLEAN for whether the password is valid. Also uses an optional
  pointer to return a bitmap of which tests failed.

  NOTE: Returns FALSE on NULL strings.

  @param[in]  String    CHAR16 pointer to the string that's being evaluated.
  @param[out] Failures  [Optional] Pointer to a UINT32 that will have bits (defined
                        in PasswordPolicyLib.h) set according to which tests may have failed.
                        If NULL, will not return a test bitmap and will fail ASAP.

  @retval     TRUE      Password is valid. "Failures" should be 0.
  @retval     FALSE     Password is invalid. "Failures" will have bits set for which tests failed.

**/
BOOLEAN
EFIAPI
PasswordPolicyIsPwStringValid (
  IN  CONST CHAR16          *String,
  OUT       PW_TEST_BITMAP  *Failures OPTIONAL
  )
{


  return FALSE;
} // PasswordPolicyIsPwStringValid()


/**
  Public interface for validating a password hash.

  Will run internal checks on the password hash to verify that it has a supported
  version and proper length.

  @param[in]  PasswordHash            Pointer to the buffer containing the hash
  @param[in]  PasswordHashSize        Size of the buffer containing the hash.

  @retval     EFI_SUCCESS             Requested operation has been successfully performed.
  @retval     EFI_INVALID_PARAMETER   There is something wrong with the formatting of
                                      the password hash.

**/
EFI_STATUS
EFIAPI
PasswordPolicyValidatePasswordHash (
  IN  CONST PASSWORD_HASH   PasswordHash,
  IN        UINTN           PasswordHashSize)
{

  return EFI_UNSUPPORTED;
}


/**
  Public interface for generating the password hash.

  Will run internal checks on the password before setting it. Returns an
  error if the password cannot be set.

  @param[in]  OldSalt               Pass in old PASSWORD_HASH to use the existing salt
  @param[in]  Password              Pointer to a buffer containing the clear text password.
                                    If Password == NULL, generate a no-password "hash"
  @param[out] PasswordHash          Pointer to a pointer that will contain the address of the password hash
                                    OldSalt == NULL : Version, Salt, and Key fields stored.
                                    OldSalt != NULL : Key field updated using existing Version and Salt.
  @param[out] PasswordHashSize      Pointer where to store the new has size

  @retval   EFI_SUCCESS             Requested operation has been successfully performed.
  @retval   EFI_INVALID_PARAMETER   There is something wrong with the formatting of
                                    the NewPassword.
  @retval   <other>                 Something else went wrong with the internal logic.

**/
EFI_STATUS
EFIAPI
PasswordPolicyGeneratePasswordHash (
  IN   CONST PASSWORD_HASH   OldSalt   OPTIONAL,
  IN   CONST CHAR16         *Password  OPTIONAL,
  OUT        PASSWORD_HASH  *PasswordHash,
  OUT        UINTN          *PasswordHashSize
  )
{

  return EFI_UNSUPPORTED;
} // PasswordSupportGeneratePasswordHash()
