/** @file
  This is an example DXE driver that overrides the Platform Key (PK) in UEFI firmware.
  This may be used for reference or testing purposes, but it is not intended for production use.
  Users should ensure that their platforms perform this operation securely and correctly.

  This may be used if a platform has lost its PK or needs to set a new PK for secure boot purposes.
  The code checks the current PK against a set of predefined hashes (PotentialTargetsHashes).
  If the PK does not match any of the expected hashes, it will return an error.

  This code is provided as an example and should be used with caution. It is the user's responsibility
  to ensure that the PK is set correctly and securely, as incorrect handling of the PK can lead to
  security vulnerabilities or system instability.

  Copyright (c) Microsoft Corporation.
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/
#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/SecureBootVariableLib.h>
#include <Library/PlatformPKProtectionLib.h>
#include <Library/BaseCryptLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BaseCryptLib.h>
#include <Library/MemoryAllocationLib.h>

#include <Guid/ImageAuthentication.h>
#include <Guid/VariableFormat.h>
#include "PlatformKey.h"

/**
 * @brief Checks if the existing Platform Key (PK) matches any of the expected hashes.
 *
 * This function retrieves the current Platform Key (PK) from the system, extracts the certificate
 * from the signature list, calculates its SHA256 hash, and compares it against a predefined list
 * of expected hashes (PotentialTargetsHashes). If the PK does not exist or does not match any of
 * the expected hashes, an appropriate EFI error is returned.
 *
 * @retval EFI_SUCCESS           The existing PK matches one of the expected hashes.
 * @retval EFI_NOT_FOUND         The PK does not exist (may indicate secure boot is disabled).
 * @retval EFI_ABORTED           The PK does not match any of the expected hashes.
 * @retval EFI_INVALID_PARAMETER The PK data structure is malformed or invalid.
 * @retval EFI_OUT_OF_RESOURCES  Memory allocation failed.
 */
EFI_STATUS
CheckPlatformKeyHash (
  )
{
  EFI_STATUS          Status                 = EFI_ABORTED;
  UINT8               *CurrentPlatformKey    = NULL;
  UINTN               CurrentPlatformKeySize = 0;
  UINT8               CalculatedHash[SHA256_DIGEST_SIZE];
  UINT8               *HashContext = NULL;
  EFI_SIGNATURE_LIST  *SigList;
  EFI_SIGNATURE_DATA  *SigData;
  UINTN               CertSize;
  UINT8               *Cert;

  Status = gRT->GetVariable (
                  EFI_PLATFORM_KEY_NAME,
                  &gEfiGlobalVariableGuid,
                  NULL,
                  &CurrentPlatformKeySize,
                  NULL
                  );

  if (Status == EFI_BUFFER_TOO_SMALL) {
    CurrentPlatformKey = AllocateZeroPool (CurrentPlatformKeySize);
    if (CurrentPlatformKey == NULL) {
      ASSERT_EFI_ERROR (CurrentPlatformKey != NULL);
      goto Exit;
    }

    Status = gRT->GetVariable (
                    EFI_PLATFORM_KEY_NAME,
                    &gEfiGlobalVariableGuid,
                    NULL,
                    &CurrentPlatformKeySize,
                    CurrentPlatformKey
                    );
    if (EFI_ERROR (Status)) {
      ASSERT_EFI_ERROR (Status);
      goto Exit;
    }
  } else if (Status == EFI_NOT_FOUND) {
    //
    // This is not really an error. We may not be able to find it because secure boot is disabled.
    //
    DEBUG ((DEBUG_ERROR, "Failed to Find the Platform Key! Is Secure Boot enabled?\n"));
    goto Exit;
  } else if (EFI_ERROR (Status)) {
    ASSERT_EFI_ERROR (Status);
    goto Exit;
  }

  if (CurrentPlatformKeySize < sizeof (EFI_SIGNATURE_LIST)) {
    ASSERT (FALSE);
    Status = EFI_INVALID_PARAMETER;
    goto Exit;
  }

  SigList = (EFI_SIGNATURE_LIST *)CurrentPlatformKey;
  SigData = (EFI_SIGNATURE_DATA *)((UINT8 *)SigList + sizeof (EFI_SIGNATURE_LIST));

  if (SigList->SignatureListSize != CurrentPlatformKeySize) {
    ASSERT (FALSE);
    Status = EFI_INVALID_PARAMETER;
    goto Exit;
  }

  if (SigList->SignatureSize <= sizeof (EFI_GUID)) {
    ASSERT (FALSE);
    Status = EFI_INVALID_PARAMETER;
    goto Exit;
  }

  CertSize = SigList->SignatureSize - sizeof (EFI_GUID);
  Cert     = (UINT8 *)SigData->SignatureData;

  if ((Cert == NULL) || (CertSize == 0)) {
    ASSERT (FALSE);
    Status = EFI_INVALID_PARAMETER;
    goto Exit;
  }

  HashContext = AllocateZeroPool (Sha256GetContextSize ());
  if (HashContext == NULL) {
    ASSERT (HashContext != NULL);
    goto Exit;
  }

  if (!Sha256Init (HashContext)) {
    ASSERT (FALSE);
    goto Exit;
  }

  if (!Sha256Update (HashContext, Cert, CertSize)) {
    ASSERT (FALSE);
    goto Exit;
  }

  if (!Sha256Final (HashContext, CalculatedHash)) {
    ASSERT (FALSE);
    goto Exit;
  }

  for (UINTN i = 0; i < POTENTIAL_TARGETS; i++) {
    DEBUG ((DEBUG_ERROR, "Expected Hash:\n"));
    DUMP_HEX (DEBUG_ERROR, 0, PotentialTargetsHashes[i], sizeof (PotentialTargetsHashes[i]), "");
    if (CompareMem (CalculatedHash, PotentialTargetsHashes[i], SHA256_DIGEST_SIZE) == 0) {
      DEBUG ((DEBUG_INFO, "Platform Key matches expected hash %u\n", i));
      Status = EFI_SUCCESS;
      goto Exit;
    }
  }

  Status = EFI_ABORTED;

Exit:
  DEBUG ((DEBUG_ERROR, "Exiting!\n"));

  if (CurrentPlatformKey != NULL) {
    FreePool (CurrentPlatformKey);
  }

  if (HashContext != NULL) {
    FreePool (HashContext);
  }

  return Status;
}

/**
 * @brief Entry point for the OverridePlatformKey DXE driver.
 *
 * This driver checks if the current Platform Key (PK) matches one of the expected hashes.
 * If it matches, the driver will delete the existing PK and install a new one.
 * After successfully updating the PK, the system will perform a warm reset.
 *
 * @param[in] ImageHandle  The firmware allocated handle for the EFI image.
 * @param[in] SystemTable  A pointer to the EFI System Table.
 *
 * @retval EFI_SUCCESS     The PK was successfully replaced.
 * @retval EFI_NOT_FOUND   The PK does not exist.
 * @retval EFI_ABORTED     The PK update process failed.
 * @retval Other           Various error codes from sub-functions.
 */
EFI_STATUS
EFIAPI
OverridePlatformKeyEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS  Status;
  BOOLEAN     requiresCleanup  = FALSE;
  UINT8       *NewPkPayload    = NULL;
  UINTN       NewPkPayloadSize = 0;

  EFI_TIME  Time;

  DEBUG ((DEBUG_INFO, "OverridePlatformKey DXE Driver Loaded\n"));

  //
  // Check if the current Platform Key (PK) matches the any expected hash
  //
  Status = CheckPlatformKeyHash ();
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "Platform Key does not match any expected hash: %r\n", Status));
    // As soon as the Platform Key does not match the expected hash, the driver will return an error.
    // This is to prevent the Platform Key from being deleted if it does not match the expected hash.

    // Once we have changed the Platform Key, we should never run this driver again.
    return Status;
  }

  DEBUG ((DEBUG_INFO, "Time to replace!\n"));

  //
  // Step 1: Notify that a PK update is coming shortly...
  //
  Status = DisablePKProtection ();
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a - Failed to signal PK update start! %r\n", __func__, Status));
    // Classify this as a PK deletion error.
    Status = EFI_ABORTED;
    goto Exit;
  }

  //
  // Step 2: Attempt to delete the PK.
  //
  Status = DeletePlatformKey ();
  DEBUG ((DEBUG_INFO, "%a - PK Delete = %r\n", __func__, Status));
  // If the PK is not found, then our work here is done.
  if (EFI_ERROR (Status) && (Status != EFI_NOT_FOUND)) {
    Status = EFI_ABORTED;
    goto Exit;
  }

  //
  // Step 3: Create the new PK payload.
  //
  Status = SystemTable->RuntimeServices->GetTime (&Time, NULL);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a - Failed to get current time! %r\n", __func__, Status));
    goto Exit;
  }

  NewPkPayloadSize = sizeof (NewPlatformKey);
  NewPkPayload     = AllocatePool (NewPkPayloadSize);
  CopyMem (NewPkPayload, NewPlatformKey, NewPkPayloadSize);
  //
  // CreateTimeBasedPayload will free our input pointer for us and
  //
  Status = CreateTimeBasedPayload (&NewPkPayloadSize, &NewPkPayload, &Time);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a - Failed to create time-based payload! %r\n", __func__, Status));
    goto Exit;
  }

  requiresCleanup = TRUE;

  // Step 4: Set the Platform Key
  Status = SystemTable->RuntimeServices->SetVariable (
                                           L"PK",
                                           &gEfiGlobalVariableGuid,
                                           VARIABLE_ATTRIBUTE_NV_BS_RT_AT,
                                           NewPkPayloadSize,
                                           NewPkPayload
                                           );
  ASSERT_EFI_ERROR (Status);

  //
  // Try a warm reset
  //
  SystemTable->RuntimeServices->ResetSystem (EfiResetWarm, EFI_SUCCESS, 0, NULL);

Exit:

  if (requiresCleanup && (NewPkPayload != NULL)) {
    FreePool (NewPkPayload);
  }

  return Status;
}

// Unload function for the DXE driver
EFI_STATUS
EFIAPI
OverridePlatformKeyUnload (
  IN EFI_HANDLE  ImageHandle
  )
{
  EFI_STATUS  Status = EFI_SUCCESS;

  // Your driver cleanup code here

  return Status;
}
