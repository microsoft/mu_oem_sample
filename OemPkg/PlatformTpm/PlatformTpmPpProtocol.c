/** @file -- PlatformTpmPpProtocol.c

  Copyright (c) Microsoft Corporation.
  SPDX-License-Identifier: BSD-2-Clause-Patent

  Contains initialization and business logic for the platform TPM
  Physical Presence Protocol.

**/

#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/Tcg2PpVendorLib.h>
#include <Library/Tpm2CommandLib.h>                // Tpm2Clear() and Tpm2ClearControl()
#include <Library/HobLib.h>                        // Get BootMode
#include <Library/Tcg2PhysicalPresenceLib.h>       // Tcg2PhysicalPresenceLibProcessRequest()
#include <Library/Tcg2PhysicalPresencePromptLib.h> // GUI Prompt

#include <Protocol/TpmPpProtocol.h>

#include <Guid/TpmInstance.h>
#include <Guid/Tcg2PhysicalPresenceData.h>

#include "PlatformTpmPpProtocolPrivate.h"        // Private function prototypes and the like.

extern EFI_BOOT_SERVICES     *gBS;
extern EFI_RUNTIME_SERVICES  *gRT;

BOOLEAN                           mVariablesValid = FALSE;
EFI_TCG2_PHYSICAL_PRESENCE_FLAGS  mPpiFlags;
EFI_TCG2_PHYSICAL_PRESENCE        mTcgPpData;

TPM_PP_PROTOCOL  mPlatformTpmPpProtocol = {
  PpProcessUserConfirmation         // PromptForConfirmation
};

/**
  Sets up required infrastructure for dealing with TCG2 Physical Presence.
  We do it here so that we don't need to dispatch all of the Physical Presence
  logic on every boot, only if we detect a Physical Presence Request.

  @retval     EFI_SUCCESS  PPI is initialized and ready to receive requests.
  @retval     EFI_ABORTED  BootMode indicates S4 boot mode. Skipping processing.
  @retval     Others       PPI cannot process requests and is as deactivated as possible.

**/
EFI_STATUS
InitPhysicalPresence (
  VOID
  )
{
  EFI_STATUS  Status;
  EFI_HANDLE  Handle = NULL;

  DEBUG ((DEBUG_INFO, "PlatformTpm::InitPhysicalPresence()\n"));

  // Make sure we need to initialize these things...
  // (ie. make sure TPM is enabled).
  if (CompareGuid (PcdGetPtr (PcdTpmInstanceGuid), &gEfiTpmDeviceInstanceNoneGuid)) {
    DEBUG ((DEBUG_INFO, "PlatformTpm::InitPhysicalPresence - TPM is disabled.\n"));
    return EFI_UNSUPPORTED;
  }

  //
  // Step 1: Make sure that the variables that serve as the communication
  //         between SMI and this driver are created if they don't already exist.
  // NOTE: This call will also return the current variables if they exist.
  Status = FindOrCreatePhysicalPresenceVariables (&mPpiFlags, &mTcgPpData);
  DEBUG ((DEBUG_VERBOSE, "PlatformTpm::InitPhysicalPresence - FindOrCreatePhysicalPresenceVariables() = %r.\n", Status));
  // If something happened with this part, we should REALLY take a better look at it.
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "PlatformTpm::InitPhysicalPresence - Failed to create PP infrastructure!!\n"));
    ASSERT_EFI_ERROR (Status);
    return Status;
  }

  // Acknowledge that if we haven't returned by now, the variables are valid.
  mVariablesValid = TRUE;

  // At this point...
  // We know that the infrastructure is in place and locked correctly.
  // Now we can detect whether there are pending requests and set up handlers for them.

  //
  // Step 2: If we're in an S4 boot mode, skip processing.
  if (GetBootModeHob () == BOOT_ON_S4_RESUME) {
    DEBUG ((EFI_D_INFO, "PlatformTpm::InitPhysicalPresence - S4 Resume, Skip TPM PP process!\n"));
    return EFI_ABORTED;
  }

  //
  // Step 3: Detect any pending OS requests.
  if (mTcgPpData.PPRequest != TCG2_PHYSICAL_PRESENCE_NO_ACTION) {
    DEBUG ((DEBUG_INFO, "PlatformTpm::%a - Pending PPI request detected!! Processing further.\n", __func__));
    // Attempt to install the PP Protocol.
    // This will:
    // - Act as a signal to other parts of the BIOS that a PP request is pending.
    // - Provide an interface to prompt for a PP confirmation once the necessary
    //    resources are available.
    Status = gBS->InstallProtocolInterface (
                    &Handle,
                    &gTpmPpProtocolGuid,
                    EFI_NATIVE_INTERFACE,
                    &mPlatformTpmPpProtocol
                    );

    // At this point we should attempt to report any EFI_ERROR Status to the OS through PPI.
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "PlatformTpm::InitPhysicalPresence - Failed to install PPI protocol (%r)!\n", Status));
      LogPhysicalPresenceResult (&mTcgPpData, TCG_PP_OPERATION_RESPONSE_BIOS_FAILURE);
    }
  } else {
    DEBUG ((DEBUG_INFO, "PlatformTpm::InitPhysicalPresence - No pending action detected. Skipping handler init.\n"));
  }

  // TODO: Consider eliminating the flags and permanently force PPI to require confirmation.

  return Status;
} // InitPhysicalPresence()

/**
  Check to see whether either of the required Physical Presence variables
  is missing.
  If present, return current value.
  If missing, initialize it with the correct default value.

  @retval     EFI_SUCCESS  All required variables now exist.
  @retval     Others       Something went wrong and one or more variables is not
                           initialized.

**/
STATIC
EFI_STATUS
FindOrCreatePhysicalPresenceVariables (
  EFI_TCG2_PHYSICAL_PRESENCE_FLAGS  *PpiFlags,
  EFI_TCG2_PHYSICAL_PRESENCE        *TcgPpData
  )
{
  EFI_STATUS  Status;
  UINTN       DataSize;

  DEBUG ((DEBUG_VERBOSE, "PlatformTpm::FindOrCreatePhysicalPresenceVariables()\n"));

  //
  // Initialize Physical Presence flags, if not found.
  DataSize = sizeof (EFI_TCG2_PHYSICAL_PRESENCE_FLAGS);
  Status   = gRT->GetVariable (
                    TCG2_PHYSICAL_PRESENCE_FLAGS_VARIABLE,
                    &gEfiTcg2PhysicalPresenceGuid,
                    NULL,
                    &DataSize,
                    PpiFlags
                    );
  if (Status == EFI_NOT_FOUND) {
    //    BIT5  -  TCG2_BIOS_TPM_MANAGEMENT_FLAG_PP_REQUIRED_FOR_TURN_OFF <BR>
    //    BIT6  -  TCG2_BIOS_TPM_MANAGEMENT_FLAG_PP_REQUIRED_FOR_CHANGE_EPS <BR>
    //    BIT7  -  TCG2_BIOS_TPM_MANAGEMENT_FLAG_PP_REQUIRED_FOR_CHANGE_PCRS <BR>
    PpiFlags->PPFlags = 0xE0;
    Status            = gRT->SetVariable (
                               TCG2_PHYSICAL_PRESENCE_FLAGS_VARIABLE,
                               &gEfiTcg2PhysicalPresenceGuid,
                               EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                               sizeof (EFI_TCG2_PHYSICAL_PRESENCE_FLAGS),
                               PpiFlags
                               );
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "PlatformTpm::FindOrCreatePhysicalPresenceVariables - SetVariable(TCG2_PHYSICAL_PRESENCE_FLAGS_VARIABLE) = %r\n", Status));
      return Status;
    }
  }

  //
  // Initialize Physical Presence variable, if not found.
  if (!EFI_ERROR (Status)) {
    DataSize = sizeof (EFI_TCG2_PHYSICAL_PRESENCE);
    Status   = gRT->GetVariable (
                      TCG2_PHYSICAL_PRESENCE_VARIABLE,
                      &gEfiTcg2PhysicalPresenceGuid,
                      NULL,
                      &DataSize,
                      TcgPpData
                      );
    if (Status == EFI_NOT_FOUND) {
      ZeroMem ((VOID *)TcgPpData, sizeof (EFI_TCG2_PHYSICAL_PRESENCE));
      Status = gRT->SetVariable (
                      TCG2_PHYSICAL_PRESENCE_VARIABLE,
                      &gEfiTcg2PhysicalPresenceGuid,
                      EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                      sizeof (EFI_TCG2_PHYSICAL_PRESENCE),
                      TcgPpData
                      );
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "PlatformTpm::FindOrCreatePhysicalPresenceVariables - SetVariable(TCG2_PHYSICAL_PRESENCE_VARIABLE) = %r\n", Status));
        return Status;
      }
    }
  }

  return Status;
} // FindOrCreatePhysicalPresenceVariables()

/**
  Will save the results of a PP request so that the OS query the status.

  @param[in]  TcgPpData     A pointer to the current PP data.
  @param[in]  ResponseCode  The response/result to save.

  @retval     EFI_SUCCESS   Response successfully saved and should be readable by the OS.
  @retval     Others        CRITICAL ERROR! OS may get invalid or stale response data.

**/
STATIC
EFI_STATUS
LogPhysicalPresenceResult (
  IN  EFI_TCG2_PHYSICAL_PRESENCE  *TcgPpData,
  IN  UINT32                      ResponseCode
  )
{
  EFI_STATUS  Status;

  DEBUG ((DEBUG_VERBOSE, "PlatformTpm::LogPhysicalPresenceResult()\n"));

  //
  // Initialize the data to save.
  TcgPpData->PPResponse    = ResponseCode;
  TcgPpData->LastPPRequest = TcgPpData->PPRequest;
  // Flush the pending action so that actions aren't triggered twice.
  TcgPpData->PPRequest = TCG2_PHYSICAL_PRESENCE_NO_ACTION;

  //
  // Attempt to save the data.
  Status = gRT->SetVariable (
                  TCG2_PHYSICAL_PRESENCE_VARIABLE,
                  &gEfiTcg2PhysicalPresenceGuid,
                  EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                  sizeof (EFI_TCG2_PHYSICAL_PRESENCE),
                  TcgPpData
                  );
  Status = EFI_SUCCESS;
  DEBUG ((DEBUG_VERBOSE, "PlatformTpm::LogPhysicalPresenceResult - PPResponse = 0x%X, SetVariable() = %r\n", ResponseCode, Status));

  //
  // If we attempted to report something to the OS, and failed to do so, that's pretty bad.
  //
  DEBUG_CODE (
    if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "PlatformTpm::LogPhysicalPresenceResult - Failed to notify OS of error!\n"));
    ASSERT_EFI_ERROR (Status);
  }

    );

  return Status;
} // LogPhysicalPresenceResult()

/**
  Handles all the logic to prompt the user for confirmation of a
  TPM Physical Presence request.

  NOTE: Currently, we should NOT return from this function.
        The system will reset after performing the requested action.


  @param[in]  This

  @retval Any   Since we should never return from this function, any
                return value is an anomaly.

**/
STATIC
EFI_STATUS
EFIAPI
PpProcessUserConfirmation (
  IN TPM_PP_PROTOCOL  *This
  )
{
  EFI_STATUS  Status = EFI_SUCCESS;

  DEBUG ((DEBUG_INFO, "PlatformTpm::PpProcessUserConfirmation()\n"));

  //
  // Step 1: First things first, make sure that we're dealing with good variables.
  if (!mVariablesValid) {
    DEBUG ((DEBUG_ERROR, "PlatformTpm::PpProcessUserConfirmation - Required variables were not cached correctly!\n"));
    ASSERT (FALSE);
    Status = EFI_INVALID_PARAMETER;
  }

  //
  // At this point we can get to the variables, so any failures should attempt
  // to be reported to the OS.
  //

  //
  // Step 2: Prompt the user for confirmation of the TPM Physical Presence request.
  //
  // Note that this step is specifically skipped in this implementation, as user interaction is not required.

  //
  // Step 3: Away with ye...
  //         Hand-off to the DxeTcg2PhysicalPresenceLib for standard processing.
  if (!EFI_ERROR (Status)) {
    // This returns VOID, so we can't really detect any errors at this point.
    Tcg2PhysicalPresenceLibProcessRequest (NULL);   // PlatformAuth [Optional]
  }

  // IMPORTANT NOTE!!! We don't get back here. Tcg2PhysicalPresenceLibProcessRequest() will reset the system.
  //                    Actually, that's only half-right. It resets on any TPM clear, but not on "flags" requests.
  //                    Still... solidarity!
  DEBUG ((DEBUG_INFO, "PlatformTpm::PpProcessUserConfirmation - Forcing reset!!\n"));
  gRT->ResetSystem (EfiResetCold, EFI_SUCCESS, 0, NULL);
  ASSERT (FALSE);

  // TODO: Consider informing the user (with a splash screen) when a pre-approved TPM action is occurring. Just don't require interaction.

  return Status;
} // PpProcessUserConfirmation()
