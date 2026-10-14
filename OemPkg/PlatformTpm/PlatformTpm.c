/** @file -- PlatformTpm.c

  Copyright (c) Microsoft Corporation.
  SPDX-License-Identifier: BSD-2-Clause-Patent

Primary driver entry point and business logic for the platform TPM.

**/

#include <Library/DebugLib.h>
#include <Library/BaseLib.h>
#include <Library/UefiLib.h>
#include <Library/Tpm2CommandLib.h>
#include <Library/Tpm2DeviceLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include <Guid/Tcg2PhysicalPresenceData.h>

#include <Protocol/Tcg2Protocol.h>

#include <Library/OemTpm2InitLib.h>             // Relies on Tcg2Protocol, so must come later.

#include <Protocol/ResetNotification.h>         // Use for the EFI reset callback.

EFI_EVENT  mPreReadyToBootEvent;    // These events are used to secure the TPM prior to leaving the boot environment.
EFI_EVENT  mExitBootServicesEvent;  // These events are used to secure the TPM prior to leaving the boot environment.

/**
  Sets up required infrastructure for dealing with Tcg2 Physical Presence.
  We do it here so that we don't need to dispatch all of the Physical Presence
  logic on every boot, only if we detect a Physical Presence Request.

  @retval     EFI_SUCCESS  PPI is initialized and ready to receive requests.
  @retval     Others       PPI cannot process requests and is as deactivated as possible.

**/
EFI_STATUS
InitPhysicalPresence (
  VOID
  );

VOID
EFIAPI
SecureTpmPriorToBoot (
  IN      EFI_EVENT  Event,
  IN      VOID       *Context
  );

// ==============================================================================
// PRIMARY DRIVER ENTRY POINT
// Principle initialization function for the platform TPM driver.

/**
  The driver's entry point.

  @param[in] ImageHandle  The firmware allocated handle for the EFI image.
  @param[in] SystemTable  A pointer to the EFI System Table.

  @retval EFI_SUCCESS     The entry point is executed successfully.
  @retval other           Some error occurs when executing this entry point.

**/
EFI_STATUS
EFIAPI
DriverEntry (
  IN    EFI_HANDLE        ImageHandle,
  IN    EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS  Status;

  DEBUG ((DEBUG_INFO, "PlatformTpm::DriverEntry()\n"));

  //
  // Install PriorToBoot callback for locking things down.
  // This callback will do things like disable the Platform Hierarchy so that
  // OSes cannot perform maintenance functions on the TPM.
  // We will ideally do this before ReadyToBoot, but have a fallback on ExitBootServices.
  Status = gBS->CreateEventEx (
                  EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  SecureTpmPriorToBoot,
                  NULL,
                  &gEfiEventPreReadyToBootGuid,
                  &mPreReadyToBootEvent
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a: ERROR: Failed to register for PreReadyToBoot. %r\n", __func__, Status));
  }

  DEBUG ((DEBUG_VERBOSE, "PlatformTpm::DriverEntry - EfiCreateEventReadyToBootEx() = %r\n", Status));
  Status = gBS->CreateEventEx (
                  EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  SecureTpmPriorToBoot,
                  NULL,
                  &gEfiEventExitBootServicesGuid,
                  &mExitBootServicesEvent
                  );
  DEBUG ((DEBUG_VERBOSE, "PlatformTpm::DriverEntry - CreateEventEx(ExitBootServices) = %r\n", Status));

  //
  // Initialize the Physical Presence Interface.
  // This must occur after installing the EFI callback because
  // it may reset the system, and a TPM notification would be required.
  Status = InitPhysicalPresence ();
  DEBUG ((DEBUG_VERBOSE, "PlatformTpm::DriverEntry - InitPhysicalPresence() = %r\n", Status));

  //
  // Run any device-specific initialization.
  Status = OemTpm2VendorSpecificInit ();
  DEBUG ((DEBUG_VERBOSE, "PlatformTpm::DriverEntry - Device-specific init = %r\n", Status));

  // No matter what, we need to return EFI_SUCCESS so that any callbacks
  // that WERE successfully registered stay resident. We're kinda past the
  // point where we can usefully handle errors, so the best we can do is log
  // everything we can and enable whatever code IS working.
  return EFI_SUCCESS;
} // DriverEntry()

// ==============================================================================
// CALLBACKS AND NOTIFIES
// These are event callbacks and notification functions for interacting with
// the rest of the core and securing the TPM at certain phases.

/**
  ReadyToBoot callback to secure the TPM Platform Hierarchy.
  Responsible for:
  - Setting the PH Auth to a random value.
  - Clear phEnable

  @param[in]  Event     Event whose notification function is being invoked
  @param[in]  Context   Pointer to the notification function's context

**/
VOID
EFIAPI
SecureTpmPriorToBoot (
  IN      EFI_EVENT  Event,
  IN      VOID       *Context
  )
{
  EFI_STATUS  Status;

  DEBUG ((DEBUG_INFO, "PlatformTpm::SecureTpmPriorToBoot()\n"));

  //
  // Make sure that we have use of the TPM.
  Status = Tpm2RequestUseTpm ();
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "PlatformTpm::SecureTpmPriorToBoot - Tpm2RequestUseTpm Failed! %r\n", Status));
    ASSERT_EFI_ERROR (Status);
  }

  //
  // Alright, first thing's first.
  // Let's set the PH Auth to a random value.
  // We will use this auth to disable the last few bits of the hierarchy.
  // TODO: FOR THE FUTURE...
  //    At the moment, we don't need the PH in runtime, so we're just going to disable it.
  //    Should we ever need the PH, we should follow this sequence:
  //    1) Create and install PH auth policy (decide what the criteria will be)
  //    2) Generate good random and set PH auth to random auth (make sure to purge ALL buffers that held the auth temporarily)
  //    3) Optionally disable the PH NV space, if not needed.
  //    4) Leave the PH hierarchy enabled for boot.
  // If we leave the PH on, additional steps may be required to expose the TPM to the OS so that libraries can use the PH.

  // NOTE: For consistency, it SHOULD be possible to roll this code into MsTpm2InitLib, but that may over-complicate things.
  //       Careful management of when Tcg2Dxe is allowed to register callbacks and whatnot would have to be taken.

  //
  // Let's do what we can to shut down the hierarchies.
  // Tpm2HierarchyControl()
  // @retval EFI_SUCCESS      Operation completed successfully.
  // @retval EFI_DEVICE_ERROR Unexpected device behavior.

  // Disable the PH NV.
  // IMPORTANT NOTE: We *should* be able to disable the PH NV here, but TPM parts have
  //                 been known to store the EK cert in the PH NV. If we disable it, the
  //                 EK cert will be unreadable.

  // Disable the PH.
  Status = Tpm2HierarchyControl (
             TPM_RH_PLATFORM,                        // AuthHandle
             NULL,                                   // AuthSession
             TPM_RH_PLATFORM,                        // Hierarchy
             NO                                      // State
             );
  DEBUG ((DEBUG_VERBOSE, "PlatformTpm::SecureTpmPriorToBoot - Disable PH = %r\n", Status));
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "PlatformTpm::SecureTpmPriorToBoot - Disable PH Failed! %r\n", Status));
    ASSERT_EFI_ERROR (Status);
  }

  //
  // Once we've completed the lockdown, we can close the event.
  gBS->CloseEvent (mPreReadyToBootEvent);
  gBS->CloseEvent (mExitBootServicesEvent);

  return;
} // SecureTpmPriorToBoot()
