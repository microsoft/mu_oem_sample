/** @file -- PasswordPolicyInternal.h

  Internal password structure for password hash.

  Copyright (C) Microsoft Corporation. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef _PASSWORD_POLICY_INTERNAL_H_
#define _PASSWORD_POLICY_INTERNAL_H_

#pragma pack(1)

// This structure is required to be at the top of all password entries.
typedef struct {
    UINT16          Version;
} PASSWORD_HASH_HEADER;

//
// Version 1 Definitions
//
#define PRIVATE_HASH_VER_1_VERSION          1
#define PRIVATE_HASH_VER_1_HASH_DIGEST_SIZE SHA256_DIGEST_SIZE
#define PRIVATE_HASH_VER_1_SALT_SIZE        32
#define PRIVATE_HASH_VER_1_ITERATION_COUNT  60000   // Tuned for ~0.5 sec computation.
#define PRIVATE_HASH_VER_1_KEY_SIZE         40

typedef struct {
    UINT32          DeletedHash;
} PASSWORD_HASH_DELETED;

typedef struct {
    PASSWORD_HASH_HEADER  Header;
    UINT8                 Salt[PRIVATE_HASH_VER_1_SALT_SIZE];
    UINT8                 Key[PRIVATE_HASH_VER_1_KEY_SIZE];
} PRIVATE_HASH_VER_1;

#define PRIVATE_HASH_VER_1_VERSION_SIZE    sizeof(PRIVATE_HASH_VER_1)

#pragma pack()


// Special version for Deleting a password
#define PASSWORD_HASH_VER_DELETE      0xADDEADDE        // Version reserved for deleting a password
#define PASSWORD_HASH_VER_DELETE_SIZE sizeof(UINT32)

//
// Definitions for working with password tests.
//
typedef UINT32            PW_TEST_BITMAP;
#define PW_TEST_STRING_NULL         (1 << 0)
#define PW_TEST_STRING_TOO_SHORT    (1 << 1)
#define PW_TEST_STRING_TOO_LONG     (1 << 2)
#define PW_TEST_STRING_INVALID_CHAR (1 << 3)
//
// Data Store Union - A structure large enough to hold any password store.
//
typedef union {
    UINT8                  HashBytes;
    PASSWORD_HASH_DELETED  Deleted;
    PASSWORD_HASH_HEADER   Hdr;
    PRIVATE_HASH_VER_1     Ver1;
} INTERNAL_PASSWORD_HASH;

#endif
