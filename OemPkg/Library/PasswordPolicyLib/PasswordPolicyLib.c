/** @file PasswordPolicyLib.c

  Library to support password hashes.

  Copyright (C) Microsoft Corporation. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <PiDxe.h>

#include <Protocol/MuPkcs5PasswordHash.h>
#include <Protocol/Rng.h>               // Random number generation used for SALTs.

#include <Library/BaseLib.h>
#include <Library/BaseCryptLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PasswordPolicyLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include "PasswordPolicyInternal.h"

typedef struct {
  UINT16  HashVersion;
  UINTN   HashSize;
} MS_PASSWORD_HASH;

STATIC MS_PASSWORD_HASH mPasswordHashVersions[] = {
  { PRIVATE_HASH_VER_1_VERSION, sizeof(PRIVATE_HASH_VER_1) }
};

// A string containing all valid characters.
STATIC CHAR16  *ValidChars = L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()?<>{}[]-_=+|.,;:'`~\"";

STATIC MU_PKCS5_PASSWORD_HASH_PROTOCOL *mPkcs5Protocol = NULL;

/**
  Determines whether a single character is valid.

  CURRENT IMPLEMENTATION:
    Search for character in valid char string.
    Return TRUE if found.

  @param[in]  Char    Character being evaluated.

  @retval     TRUE    Character is valid for a password.
  @retval     FALSE   Not.

**/
STATIC
BOOLEAN
IsValidChar (
  IN  CHAR16    Char
  )
{
  BOOLEAN Result = FALSE;
  UINTN   ValidCharsCount, Index;

  // Assume FALSE unless the character is found.

  // Search all valid characters for the character in question.
  ValidCharsCount = StrLen( ValidChars );
  for (Index = 0; Index < ValidCharsCount; Index++)
  {
    if (Char == ValidChars[Index])
    {
      Result = TRUE;
      break;
    }
  }
  return Result;
} // IsValidChar()

/**
  This helper function will lookup and return the correct parameters
  for any version of the password store.

  NOTE: Any of the parameter pointers may be NULL. If so, they will be skipped.
  NOTE: This function should ALWAYS stay static. Callers need to be able to trust this data implicitly,
        as though they had set the values themselves, so let's keep it to one file. Create a wrapper
        interface if an external caller needs to consume this.

  @param[in]  Version     The version of the parameters being requested.
  @param[out] DigestSize  [Optional] Pointer used to return the correct digest size.
  @param[out] SaltSize    [Optional] Pointer used to return the correct salt size.
  @param[out] KeySize     [Optional] Pointer used to return the correct key size.
  @param[out] IterationCount  [Optional] Pointer used to return the correct iteration count.

  @retval     EFI_SUCCESS             Requested information has been returned.
  @retval     EFI_INVALID_PARAMETER   Unknown version requested.

**/
STATIC
EFI_STATUS
GetPasswordStoreParameters (
  IN  UINT16    Version,
  OUT UINTN     *DigestSize OPTIONAL,
  OUT UINTN     *SaltSize OPTIONAL,
  OUT UINTN     *KeySize OPTIONAL,
  OUT UINTN     *IterationCount OPTIONAL
  )
{

  DEBUG((DEBUG_INFO, "%a: Entry\n", __FUNCTION__));

  //
  // Right off the bat, bail if we don't recognize the version.
  // TODO: Update this test to include future versions if they're created.
  if (Version != PRIVATE_HASH_VER_1_VERSION)
  {
    return EFI_INVALID_PARAMETER;
  }

  //
  // For now, we only have the one version, so we can
  // quickly return these values if requested.
  if (DigestSize) *DigestSize         = PRIVATE_HASH_VER_1_HASH_DIGEST_SIZE;
  if (SaltSize) *SaltSize             = PRIVATE_HASH_VER_1_SALT_SIZE;
  if (KeySize) *KeySize               = PRIVATE_HASH_VER_1_KEY_SIZE;
  if (IterationCount) *IterationCount = PRIVATE_HASH_VER_1_ITERATION_COUNT;

  return EFI_SUCCESS;
} // GetPasswordStoreParameters()

/**
  Uses a pre-defined algorithm to generate a random SALT for
  password hashing.

  @param[out]  SaltBuffer     Pointer to the buffer where the SALT goes.
  @param[in]   SaltBufferSize Size of the SALT buffer and number of random bytes requested.

  @retval     TRUE    SALT generated and placed in SaltBuffer.
  @retval     FALSE   SALT generation failed. Most likely due to insufficient entropy.

**/
STATIC
BOOLEAN
GenerateSalt (
  OUT  UINT8    *SaltBuffer,
  IN   UINTN    SaltBufferSize
  )
{
  EFI_STATUS        Status;
  EFI_RNG_PROTOCOL  *EfiRngProtocol;

  DEBUG((DEBUG_INFO, "%a: Entry\n", __FUNCTION__));

  //
  // SECURITY NOTE:
  //  This currently leverages the RngProtocol -- which, in turn, leverages RDRAND -- as its
  //  SOLE source of random. Before going to production, this method should be switched to a
  //  shared method that pools entropy from multiple sources, rather than a single source.
  //

  //
  // Step 1: Locate the RNG protocol.
  Status = gBS->LocateProtocol (&gEfiRngProtocolGuid, NULL, (VOID**)&EfiRngProtocol);

  //
  // Step 2: Attempt to generate the random.
  if (!EFI_ERROR (Status)) {
    Status = EfiRngProtocol->GetRNG (EfiRngProtocol,                      // This
                                     &gEfiRngAlgorithmSp80090Ctr256Guid,  // RNGAlgorithm
                                     SaltBufferSize,                      // RNGValueLength
                                     SaltBuffer                           // RNGValue
                                     );
    if (!EFI_ERROR(Status)) {
      return TRUE;
    }

    Status = EfiRngProtocol->GetRNG (EfiRngProtocol,                       // This
                                     &gEfiRngAlgorithmSp80090Hmac256Guid,  // RNGAlgorithm
                                     SaltBufferSize,                       // RNGValueLength
                                     SaltBuffer                            // RNGValue
                                     );
    if (!EFI_ERROR(Status)) {
      return TRUE;
    }

    Status = EfiRngProtocol->GetRNG (EfiRngProtocol,                       // This
                                     &gEfiRngAlgorithmSp80090Hash256Guid,  // RNGAlgorithm
                                     SaltBufferSize,                       // RNGValueLength
                                     SaltBuffer                            // RNGValue
                                     );
  }

  return !EFI_ERROR (Status);
} // GenerateSalt()

/**
  This function will produce a full, correctly formatted password store buffer.

  IMPORTANT: Password must be copied to a fresh buffer by SafeCopyPassword().
             This function does not perform any bounds checking on the password.

  @param[in]  OldStore    If present, OldStore Salt is used, otherwise, a new Salt is generated
  @param[in]  Store       A pointer to an empty VER_1 hash structure to be populated.
  @param[in]  Password    A pointer to the password buffer to be operated upon.

  @retval     EFI_SUCCESS           Everything's groovy! Your password store has been built.
  @retval     EFI_INVALID_PARAMETER The version was not recognized. Returned by GetPasswordStoreParameters().
  @retval     EFI_OUT_OF_RESOURCES  There was insufficient entropy to generate the SALT.
  @retval     EFI_ABORTED           Password was too long.
  @retval     Others                Error was returned from Pkcs5HashPassword().

**/
STATIC
EFI_STATUS
BuildV1PasswordStore (
  IN       INTERNAL_PASSWORD_HASH *OldStore OPTIONAL,
  IN       INTERNAL_PASSWORD_HASH *Store,
  IN CONST CHAR16                 *Password
  )
{
  EFI_STATUS                Status = EFI_SUCCESS;
  UINTN                     SaltSize;
  UINTN                     DigestSize;
  UINTN                     KeySize;
  UINTN                     IterationCount;
  UINT8                    *SaltBuffer;
  UINT8                    *KeyBuffer;
  UINTN                     PasswordSize;

  DEBUG((DEBUG_INFO, "%a: Entry\n", __FUNCTION__));

  if ((Store == NULL) || (Password == NULL))
  {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Step 1: Let's set up all the parameters.
  // NOTE: If this is an unknown version, GetPasswordStoreParameters() will return EFI_INVALID_PARAMETER.
  Status = GetPasswordStoreParameters( PRIVATE_HASH_VER_1_VERSION, &DigestSize, &SaltSize, &KeySize, &IterationCount );
  if (EFI_ERROR( Status ))
  {
    return Status;
  }

  Store->Ver1.Header.Version = PRIVATE_HASH_VER_1_VERSION;

  SaltBuffer = &Store->Ver1.Salt[0];
  KeyBuffer  = &Store->Ver1.Key[0];

  //
  // Step 2: If we need to provide the salt, let's do that now.
  //
  if (NULL == OldStore) {
    if (!GenerateSalt( SaltBuffer, SaltSize )) {
      return EFI_OUT_OF_RESOURCES;
    }
  } else {
    CopyMem (&Store->Ver1.Salt, OldStore->Ver1.Salt, SaltSize);
  }

  if (mPkcs5Protocol == NULL)
  {
    Status = gBS->LocateProtocol(
      &gMuPKCS5PasswordHashProtocolGuid,
      NULL,
      (VOID **)&mPkcs5Protocol);

    if (EFI_ERROR(Status))
    {
      DEBUG((DEBUG_ERROR, "%a Failed to locate PKCS5 Protocol.\n", __FUNCTION__));
      return Status;
    }
  }

  // First, get the number of CHARs in the password.
  PasswordSize    = StrLen( Password );
  // Now check for possible overflow.
  if ((PasswordSize * sizeof( *Password )) < PasswordSize)
  {
    DEBUG(( DEBUG_ERROR, "%a - Password is too long!\n", __FUNCTION__ ));
    return EFI_ABORTED;
  }
  PasswordSize = PasswordSize * sizeof( *Password );

  //
  // Finally, populate the key.
  ZeroMem( KeyBuffer, KeySize );
  if (!EFI_ERROR( Status ) && mPkcs5Protocol != NULL)
  {
    Status = mPkcs5Protocol->HashPassword( mPkcs5Protocol,
                                PasswordSize,       // PasswordSize
                                (CHAR8*)Password,   // Password
                                SaltSize,           // SaltSize
                                SaltBuffer,         // Salt
                                IterationCount,     // IterationCount
                                DigestSize,         // DigestSize
                                KeySize,            // OutputSize
                                KeyBuffer );        // Output
  }

  return Status;
} // BuildV1PasswordStore()

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

  DEBUG((DEBUG_INFO, "%a: Entry\n", __FUNCTION__));

  if (Buffer == NULL)
  {
    return EFI_INVALID_PARAMETER;
  }

  // Don't dally if the buffer is too small...
  if (BufferLength < (PW_MAX_LENGTH + 1))
  {
     DEBUG(( DEBUG_ERROR, "%a - Buffer too small. How did you even do that?\n", __FUNCTION__ ));
     ASSERT( FALSE );
     return EFI_BUFFER_TOO_SMALL;
 }

  //Otherwise, copy the string.
  StrnCpyS( Buffer, PW_MAX_LENGTH+1, Password, PW_MAX_LENGTH );
  Buffer[PW_MAX_LENGTH] = 0;  // Set last character as NULL, regardless.

  return EFI_SUCCESS;
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
  DEBUG((DEBUG_INFO, "%a: Entry\n", __FUNCTION__));

  ZeroMem( Data, Size );
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
  BOOLEAN   Result = TRUE;
  UINTN     StringLen = 0, Index;
  CHAR16    TempPassword[PW_MAX_LENGTH + 1];  // Maximum password length plus a NULL terminator.

  DEBUG((DEBUG_INFO, "%a: Entry\n", __FUNCTION__));

  // Initialize failures.
  if (Failures) *Failures = 0;

  // Do a quick NULL check and return immediately if true.
  if (String == NULL)
  {
    if (Failures) *Failures |= PW_TEST_STRING_NULL;
    return FALSE;
  }

  //
  // Step 1: Check for strings that are too long.
  PasswordPolicySafeCopyPassword( TempPassword,
                               ARRAY_SIZE( TempPassword ),
                               String );
  if (StrCmp( String, TempPassword ) != 0)
  {
    if (Failures) *Failures |= PW_TEST_STRING_TOO_LONG;
    Result = FALSE;
  }

  //
  // For all remaining tests, if Failures doesn't exist and
  // we already have a failure, there's no sense checking futher.
  // Without a Failures parameter, we can't return any more information
  // than a failure condition.
  //

  //
  // Step 2: Check for strings that are too short.
  if (Result || Failures)     // Run this test if either the last test passed or we want detailed results.
  {
    StringLen = StrLen( TempPassword );
    if (StringLen < PW_MIN_LENGTH)
    {
      if (Failures) *Failures |= PW_TEST_STRING_TOO_SHORT;
      Result = FALSE;
    }
  }

  //
  // Step 3: Make sure that the string does not contain invalid characters.
  if (Result || Failures)     // Run this test if either the last test passed or we want detailed results.
  {
    for (Index = 0; Index < StringLen; Index++)
    {
      if (!IsValidChar( TempPassword[Index] ))
      {
        if (Failures) *Failures |= PW_TEST_STRING_INVALID_CHAR;
        Result = FALSE;
        break;
      }
    }
  }

  // Always put away your toys.
  PasswordPolicyCleansePwBuffer( TempPassword, sizeof( TempPassword ) );

  return Result;
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
  CONST INTERNAL_PASSWORD_HASH *PwdHash;
        UINTN                   i;
        EFI_STATUS              Status;

  DEBUG((DEBUG_INFO, "%a: Entry\n", __FUNCTION__));

  Status = EFI_INVALID_PARAMETER;
  if ((NULL == PasswordHash) ||
      (PasswordHashSize < sizeof(PASSWORD_HASH_HEADER)))
  {
    return Status;
  }

  PwdHash = (INTERNAL_PASSWORD_HASH *) PasswordHash;
  if ((PasswordHashSize >= PASSWORD_HASH_VER_DELETE_SIZE) &&
      (PwdHash->Deleted.DeletedHash == PASSWORD_HASH_VER_DELETE))
  {
    return EFI_SUCCESS;
  }

  for (i = 0; i < ARRAY_SIZE(mPasswordHashVersions); i++)
  {
    if (PwdHash->Hdr.Version == mPasswordHashVersions[i].HashVersion)
    {
      if (PasswordHashSize == mPasswordHashVersions[i].HashSize)
      {
        Status = EFI_SUCCESS;
      }

      break;
    }
  }

  if (EFI_ERROR(Status))
  {
    DEBUG((DEBUG_ERROR, "Password store has incorrect version (0x%X). Cannot Apply new password \n", PwdHash->Hdr.Version));
  }
  return Status;
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
  EFI_STATUS              Status = EFI_SUCCESS;
  INTERNAL_PASSWORD_HASH *PwdHash;
  INTERNAL_PASSWORD_HASH *OldStore;

  DEBUG((DEBUG_INFO, "%a: Entry\n", __FUNCTION__));

  // Make sure that it's a valid password.
  if ((PasswordHash == NULL) ||
      (PasswordHashSize == NULL) ||
      ((Password == NULL) && (OldSalt != NULL)) || // OldSalt cannot be present if Password == NULL
      ((Password != NULL) && !PasswordPolicyIsPwStringValid( Password, NULL )))
  {
    Status = EFI_INVALID_PARAMETER;
    goto Exit;
  }

  if (Password == NULL)
  {
    PwdHash = (INTERNAL_PASSWORD_HASH *) AllocatePool (sizeof (PASSWORD_HASH_VER_DELETE_SIZE));
    if (NULL == PwdHash)
    {
      Status = EFI_OUT_OF_RESOURCES;
    }

    PwdHash->Deleted.DeletedHash = PASSWORD_HASH_VER_DELETE;
    *PasswordHash = &PwdHash->HashBytes;
    *PasswordHashSize = PASSWORD_HASH_VER_DELETE_SIZE;
    Status = EFI_SUCCESS;
    goto Exit;
  }

  OldStore = (INTERNAL_PASSWORD_HASH *) OldSalt;

  if (NULL != OldStore) {
    if (OldStore->Ver1.Header.Version != PRIVATE_HASH_VER_1_VERSION) {
      Status = EFI_INVALID_PARAMETER;   // Currently, only support V1 password hash.
      goto Exit;
    }
  }

  PwdHash = (INTERNAL_PASSWORD_HASH *) AllocateZeroPool (PRIVATE_HASH_VER_1_VERSION_SIZE);
  if (NULL == PwdHash)
  {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  // Now build the store.
  Status = BuildV1PasswordStore( OldStore,         // Old Salt? (Old PwdHash has old Salt)
                                 PwdHash,          // Store
                                 Password);        // Password
  if (EFI_ERROR( Status ))
  {
    Status = EFI_ABORTED;
    goto Exit;
  }

  *PasswordHash = &PwdHash->HashBytes;
  *PasswordHashSize = sizeof(PRIVATE_HASH_VER_1);

Exit:
  DEBUG((DEBUG_INFO, "%a: Exit. Code=%r\n", __FUNCTION__, Status));

  return Status;
} // PasswordSupportGeneratePasswordHash()
