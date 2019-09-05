/** @file SecureBootModeSetting.c

  Copyright (C) Microsoft Corporation. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent

  Library to support Secure Boot Settings  (get/set)

**/


#include <PiDxe.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/MsSecureBootLib.h>

#include <DfciSystemSettingTypes.h>

#include <Protocol/DfciSettingsProvider.h>

#include <Settings/DfciSettings.h>
#include <Settings/FrontPageSettings.h>

#define MS_SB_CONFIG_CUSTOM 3

EFI_EVENT         mSecureBootSettingsProviderSupportInstallEvent;
VOID             *mSecureBootSettingProviderSupportInstallEventRegistration = NULL;

/**
Returns the default Value of Secure boot mode
**/
EFI_STATUS
EFIAPI
SecureBootModeGetDefault(
IN  CONST DFCI_SETTING_PROVIDER     *This,
IN  OUT   UINTN                     *ValueSize,
OUT       VOID                      *Value
)
{
    if ((This == NULL) || (Value == NULL) || (ValueSize == NULL) || (This->Id == NULL))
    {
        return EFI_INVALID_PARAMETER;
    }

    if (*ValueSize < sizeof(UINT8))
    {
        *ValueSize = sizeof(UINT8);
        return EFI_BUFFER_TOO_SMALL;
    }

    if (0 != AsciiStrnCmp (This->Id, DFCI_SETTING_ID__SECURE_BOOT_KEYS_ENUM, DFCI_MAX_ID_LEN))
    {
        DEBUG((DEBUG_ERROR, "Securebootmode was called with incorrect Provider Id (%a)\n", This->Id));
        return EFI_UNSUPPORTED;
    }

    *ValueSize = sizeof(UINT8);
    *((UINT8 *) Value) = MS_SB_CONFIG_MS_3P;
    return EFI_SUCCESS;
}

/**
Returns the Current Value of secure boot mode
**/
EFI_STATUS
EFIAPI
SecureBootModeGet(
IN  CONST DFCI_SETTING_PROVIDER     *This,
IN  OUT   UINTN                     *ValueSize,
OUT       VOID                      *Value
)
{
    UINTN current;

    if ((This == NULL) || (Value == NULL) || (ValueSize == NULL) || (This->Id == NULL))
    {
        return EFI_INVALID_PARAMETER;
    }
    if (*ValueSize < sizeof(UINT8))
    {
        *ValueSize = sizeof(UINT8);
        return EFI_BUFFER_TOO_SMALL;
    }

    if (0 != AsciiStrnCmp (This->Id, DFCI_SETTING_ID__SECURE_BOOT_KEYS_ENUM, DFCI_MAX_ID_LEN))
    {
        DEBUG((DEBUG_ERROR, "TpmEnableGet was called with incorrect Provider Id (%a)\n", This->Id));
        return EFI_UNSUPPORTED;
    }

    //mIsValid
    current = GetCurrentSecureBootConfig();
    if (current == (UINTN)-1){
        *((UINT8 *) Value) = MS_SB_CONFIG_CUSTOM; // something needs to be defined for custom which might be used in the front page to display a custom string. (anything that is not None, MS, or MS_3P?
    }
    else{
        *((UINT8 *) Value) = (UINT8)current;
    }
    *ValueSize = sizeof(UINT8);
    return EFI_SUCCESS;
}
/**
Set the new mode
**/
EFI_STATUS
EFIAPI
SecureBootModeSet(
IN  CONST DFCI_SETTING_PROVIDER     *This,
IN        UINTN                      ValueSize,
IN  CONST VOID                      *Value,
OUT DFCI_SETTING_FLAGS              *Flags
)
{
    UINTN     current;
    EFI_STATUS  Status = EFI_SUCCESS;
    BOOLEAN     UseThirdParty;
    *Flags = 0;

    if (This == NULL || Value == NULL || Flags == NULL || This->Id == NULL)
    {
        return EFI_INVALID_PARAMETER;
    }

    if (ValueSize != sizeof( UINT8 )) {
      return EFI_BAD_BUFFER_SIZE;
    }

    if (0 != AsciiStrnCmp (This->Id, DFCI_SETTING_ID__SECURE_BOOT_KEYS_ENUM, DFCI_MAX_ID_LEN))
    {
        DEBUG((DEBUG_ERROR, "TpmEnableSet was called with incorrect Provider Id (%a)\n", This->Id));
        return EFI_UNSUPPORTED;
    }

    DEBUG((DEBUG_INFO, "Setting Secure Boot Mode to %d\n", *((UINT8 *) Value)));

    if (*((UINT8 *) Value) > MS_SB_CONFIG_NONE){
        DEBUG((DEBUG_ERROR, "Custom secure boot modes are not supported (%a)\n", This->Id));
        return EFI_INVALID_PARAMETER;
    }
    current = GetCurrentSecureBootConfig();
    //check current value to make sure we need to set anything. Is this applicable for secure boot??
    if (current == *((UINT8 *) Value))
    {
        *Flags |= DFCI_SETTING_FLAGS_OUT_ALREADY_SET;
        return EFI_SUCCESS;
    }
    // this is where you move what front page did.
    //
    // Take action on the SB variables.
    if (MS_SB_CONFIG_MS_ONLY == *((UINT8 *) Value) || MS_SB_CONFIG_MS_3P == *((UINT8 *) Value))
    {
        // Make sure to clear out any variables that may already be set.
        Status = DeleteSecureBootVariables();

        // If we're still good to go, do the do.
        if (!EFI_ERROR(Status))
        {
            UseThirdParty = (MS_SB_CONFIG_MS_3P == *((UINT8 *) Value));
            Status = SetDefaultSecureBootVariables(UseThirdParty);
            DEBUG((DEBUG_INFO, "INFO %a - SetDefaultSecureBootVariables(%d) = %r\n", __FUNCTION__, UseThirdParty, Status));
        }
    }
    else if (MS_SB_CONFIG_NONE == *((UINT8 *) Value))
    {
        Status = DeleteSecureBootVariables();
        DEBUG((DEBUG_INFO, "INFO %a - DeleteSecureBootVariables() = %r\n", __FUNCTION__, Status));
    }
    else
    {
        DEBUG((DEBUG_ERROR, "ERROR %a - Unknown configuration selected! 0x%X\n", __FUNCTION__, *((UINT8 *) Value)));
        Status = EFI_ABORTED;
    }

    // NOTE: Until we have an IgnoreOnce interface for VariablePolicy, we should always flag a reboot.
    // if (!EFI_ERROR(Status)){
        *Flags |= DFCI_SETTING_FLAGS_OUT_REBOOT_REQUIRED;
    // }

    return Status;
}

/**
Sets the default Value of Secure boot mode
**/
EFI_STATUS
EFIAPI
SecureBootModeSetDefault(
  IN  CONST DFCI_SETTING_PROVIDER     *This
  )
{
  EFI_STATUS Status;
  UINT8 Value = 0;
  UINTN ValueSize;
  DFCI_SETTING_FLAGS Flags = 0;
  if ((This == NULL) || (This->Id == NULL))
  {
    return EFI_INVALID_PARAMETER;
  }

  if (0 != AsciiStrnCmp (This->Id, DFCI_SETTING_ID__SECURE_BOOT_KEYS_ENUM, DFCI_MAX_ID_LEN))
  {
    DEBUG((DEBUG_ERROR, "Securebootmode was called with incorrect Provider Id (0x%X)\n", This->Id));
    return EFI_UNSUPPORTED;
  }

  ValueSize = sizeof(Value);
  Status = SecureBootModeGetDefault(This, &ValueSize, &Value);
  if (!EFI_ERROR(Status))
  {
    Status = SecureBootModeSet(This, sizeof(Value), &Value, &Flags);
  }
  return Status;
}

DFCI_SETTING_PROVIDER mProvider = {
    DFCI_SETTING_ID__SECURE_BOOT_KEYS_ENUM,
    DFCI_SETTING_TYPE_SECUREBOOTKEYENUM,
    DFCI_SETTING_FLAGS_OUT_REBOOT_REQUIRED,
    SecureBootModeSet,
    SecureBootModeGet,
    SecureBootModeGetDefault,
    SecureBootModeSetDefault
};

/*
Library design is such that a dependency on gDfciSettingsProviderSupportProtocolGuid
is not desired.  So to resolve that a ProtocolNotify is used.

This function gets triggered once on install and 2nd time when the Protocol gets installed.

When the gDfciSettingsProviderSupportProtocolGuid protocol is available the function will
loop thru all supported device disablement supported features (using PCD) and install the settings

Context is NULL.
*/
VOID
EFIAPI
SecureBootSettingProviderSupportProtocolNotify(
IN  EFI_EVENT       Event,
IN  VOID            *Context
)
{
    EFI_STATUS Status;
    DFCI_SETTING_PROVIDER_SUPPORT_PROTOCOL *sp;
    STATIC UINT8 CallCount = 0;

    //locate protocol
    Status = gBS->LocateProtocol(&gDfciSettingsProviderSupportProtocolGuid, NULL, (VOID**)&sp);
    if (EFI_ERROR(Status))
    {
      if ((CallCount++ != 0) || (Status != EFI_NOT_FOUND))
      {
        DEBUG((DEBUG_ERROR, "%a() - Failed to locate gDfciSettingsProviderSupportProtocolGuid in notify.  Status = %r\n", __FUNCTION__, Status));
      }
      return;
    }

    //call function
    DEBUG((DEBUG_INFO, "Registering Secureboot Setting Provider\n"));
    Status = sp->RegisterProvider(sp, &mProvider);
    if (EFI_ERROR(Status))
    {
        DEBUG((DEBUG_ERROR, "Failed to Register.  Status = %r\n", Status));
    }

    //We got here, this means all protocols were installed and we didn't exit early.
    //close the event as we dont' need to be signaled again. (shouldn't happen anyway)
    gBS->CloseEvent(Event);
}

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
MsSecureBootModeSettingDxeLibConstructor
(
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{

  if (FeaturePcdGet(PcdSettingsManagerInstallProvider))
  {
      //Install callback on the SettingsManager gDfciSettingsProviderSupportProtocolGuid protocol
      mSecureBootSettingsProviderSupportInstallEvent = EfiCreateProtocolNotifyEvent(
                                                               &gDfciSettingsProviderSupportProtocolGuid,
                                                               TPL_CALLBACK,
                                                               SecureBootSettingProviderSupportProtocolNotify,
                                                               NULL,
                                                               &mSecureBootSettingProviderSupportInstallEventRegistration
                                                               );

    DEBUG((DEBUG_INFO, "%a - Event Registered.\n", __FUNCTION__));
  }
  return EFI_SUCCESS;
}