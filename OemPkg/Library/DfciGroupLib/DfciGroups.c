/** @file DfciGroups.c

Library Instance for Dfci to establish platform settings that are part of Dfci Group settings.

  Copyright (C) Microsoft Corporation. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Uefi.h>

#include <DfciSystemSettingTypes.h>

#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/DfciGroupLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include <Settings/BootMenuSettings.h>
#include <Settings/DfciSettings.h>


STATIC DFCI_SETTING_ID_STRING mExternalMediaSettings[] = {
    DFCI_SETTING_ID__ENABLE_USB_BOOT,
    NULL
};

STATIC DFCI_GROUP_ENTRY mMyGroups[] = {
    { DFCI_STD_SETTING_ID__EXTERNAL_MEDIA,  (DFCI_SETTING_ID_STRING *) &mExternalMediaSettings },
    { NULL,                                 NULL }
};

/**
 * Return a pointer to the Group Array to DFCI
 *
 */
DFCI_GROUP_ENTRY *
EFIAPI
DfciGetGroupEntries (VOID) {

    return (DFCI_GROUP_ENTRY *) &mMyGroups;
}
