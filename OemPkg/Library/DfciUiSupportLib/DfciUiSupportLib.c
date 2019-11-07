/* @file DfciUiSupportLib.c

Library Instance for UI support functions for DFCI.

  Copyright (C) Microsoft Corporation. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/
#include <Uefi.h>
#include <DfciSystemSettingTypes.h>

#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/DfciUiSupportLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/SwmDialogsLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include <Protocol/OnScreenKeyboard.h>
#include <Protocol/SimpleWindowManager.h>


/**
 * DfciUiDisplayDfciAuthDialog
 *
 * @param TitleText
 * @param CaptionText
 * @param BodyText
 * @param CertText
 * @param ConfirmText
 * @param ErrorText
 * @param PasswordType
 * @param Thumbprint
 * @param Result
 * @param OPTIONAL
 *
 * @return EFI_STATUS EFIAPI
 */
EFI_STATUS
EFIAPI
DfciUiDisplayAuthDialog (
  IN  CHAR16                              *TitleText,
  IN  CHAR16                              *CaptionText,
  IN  CHAR16                              *BodyText,
  IN  CHAR16                              *CertText,
  IN  CHAR16                              *ConfirmText,
  IN  CHAR16                              *ErrorText,
  IN  BOOLEAN                             PasswordType,
  IN  CHAR16                              *Thumbprint,
  OUT DFCI_MB_RESULT                      *Result,
  OUT CHAR16                              **Password OPTIONAL
  )
{
    EFI_STATUS  Status;
    CHAR16     *ThumbprintFromUser;

    if (DfciUiIsManufacturingMode()) {
        *Result = DFCI_MB_IDOK;
        return EFI_SUCCESS;
    }

    ThumbprintFromUser = NULL;

    Status = SwmDialogsVerifyThumbprintPrompt (
                 TitleText,
                 CaptionText,
                 BodyText,
                 CertText,
                 ConfirmText,
                 ErrorText,
                 PasswordType ? SWM_THMB_TYPE_ALERT_PASSWORD : SWM_THMB_TYPE_ALERT_THUMBPRINT,
                 (SWM_MB_RESULT*)Result,
                 Password,
                &ThumbprintFromUser);

    if (!EFI_ERROR(Status)) {
        if (*Result == DFCI_MB_IDOK) {
            if (NULL == ThumbprintFromUser) {
                DEBUG((DEBUG_ERROR, "%a: Failed to get Thumbprint from Dialog"));
                *Result = DFCI_MB_IDTRYAGAIN;
                ASSERT(ThumbprintFromUser != NULL);
            }  else {
                if (StrCmp(ThumbprintFromUser, Thumbprint) != 0) {
                    *Result = DFCI_MB_IDTRYAGAIN;
                }
                FreePool (ThumbprintFromUser);
            }
        }
    }
    return Status;
}

EFI_STATUS
EFIAPI
DfciUiDisplayPasswordDialog (
  IN  CHAR16                              *TitleText,
  IN  CHAR16                              *CaptionText,
  IN  CHAR16                              *BodyText,
  IN  CHAR16                              *ErrorText,
  OUT DFCI_MB_RESULT                      *Result,
  OUT CHAR16                              **Password
  )
{

    return SwmDialogsPasswordPrompt (TitleText,
                                     CaptionText,
                                     BodyText,
                                     ErrorText,
                                     SWM_PWD_TYPE_ALERT_PASSWORD,
                                     (SWM_MB_RESULT*)Result,
                                     Password);
}

EFI_STATUS
EFIAPI
DfciUiDisplayMessageBox (
    IN  CHAR16          *TitleBarText,
    IN  CHAR16          *Text,
    IN  CHAR16          *Caption,
    IN  UINT32          Type,
    IN  UINT64          Timeout,
    OUT DFCI_MB_RESULT   *Result
    )
{

    return SwmDialogsMessageBox (TitleBarText,
                                 Text,
                                 Caption,
                                 Type,
                                 Timeout,
                                 (SWM_MB_RESULT *) Result);
}

/**
  This routine indicates if the system is in Manufacturing Mode.

  @retval  ManufacturingMode - Platforms may have a manufacturing mode.
                               DFCI Auto opt-in's the management cert included
                               in the firmware volume in Manufacturing Mode.
                               TRUE if the device is in Manufacturing Mode
**/
BOOLEAN
EFIAPI
DfciUiIsManufacturingMode (
  VOID
  ) {
    return FALSE;
}

/**

  This routine indicates if the UI is ready and can be used.

  @retval  TRUE if the UI is ready to use, else FALSE.

**/
BOOLEAN
EFIAPI
DfciUiIsUiAvailable (
  VOID
  )
{
    EFI_STATUS  Status;
    VOID       *Ptr;

    // Locate the Simple Window Manager protocol.
    //
    Status = gBS->LocateProtocol (&gMsSWMProtocolGuid,
                                  NULL,
                                  &Ptr);

    return (Status == EFI_SUCCESS);
}


/**
    DfciUiExitSecurityBoundary

    UEFI that support locked settings variables can lock those
    variable when this function is called.  DFCI will call this function
    before enabling USB or the Network device which are considered unsafe.

    Signal PreReadyToBoot - lock private settings variable to ensure
           USB or Network don't have access to locked settings.
    Disable the OSK from being displayed (PreReadyToBoot also enables the OSK)
**/
VOID
EFIAPI
DfciUiExitSecurityBoundary (VOID) {

    UINT32                         OSKMode;
    MS_ONSCREEN_KEYBOARD_PROTOCOL *OSKProtocol;
    EFI_STATUS                     Status;


    // Platform Late Locking event.  For now, just signal
    // PreReadyToBoot() to exit the trust boundary.
    //
    // All exits from now on should restart the system to get back
    // inside the security boundary to continue making changes.
    EfiEventGroupSignal (&gEfiEventPreReadyToBootGuid);

    // This also enables auto keyboard enable, so turn it back off:

    // Locate the on-screen keyboard (OSK) protocol.  It may be used for input on a touch-only device.
    //
    OSKProtocol = NULL;
    Status = gBS->LocateProtocol (&gMsOSKProtocolGuid,
                                  NULL,
                                  (VOID **)&OSKProtocol
                                 );
    if (EFI_ERROR (Status))
    {
        DEBUG((DEBUG_WARN, "%a: Failed to locate on-screen keyboard protocol (%r).\r\n", __FUNCTION__, Status));
        OSKProtocol = NULL;
    }

    if (NULL != OSKProtocol)
    {
        // Disable OSK icon auto-activation and self-refresh, and ensure keyboard is disabled.
        //
        OSKProtocol->GetKeyboardMode(OSKProtocol, &OSKMode);
        OSKMode &= ~(OSK_MODE_AUTOENABLEICON | OSK_MODE_SELF_REFRESH);
        OSKProtocol->ShowKeyboard(OSKProtocol, FALSE);
        OSKProtocol->ShowKeyboardIcon(OSKProtocol, FALSE);
        OSKProtocol->SetKeyboardMode(OSKProtocol, OSKMode);
    }
}
