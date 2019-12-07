/** @file
  Header file to support the Reboot Reason non volatile variables

  Copyright (C) Microsoft Corporation. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef _MS_NV_BOOT_REASON_H_
#define _MS_NV_BOOT_REASON_H_

//==========================================
// Data Structure GUID and Definitions
//==========================================

// {9B09B8C1-3CC0-4F3B-A6CB-A0214B7F4C63}
#define OEM_BOOT_NV_VAR_GUID \
  { \
    0x9b09b8c1, 0x3cc0, 0x4f3b, { 0xa6, 0xcb, 0xa0, 0x21, 0x4b, 0x7f, 0x4c, 0x63 }\
  }

extern EFI_GUID gOemBootNVVarGuid;


//==========================================
// Event GUIDs and Definitions
//==========================================

//
// Create definitions for the FrontPage variables.
#define SFP_NV_SETTINGS_VAR_NAME          L"FPConfigNVData"
#define SFP_NV_ATTRIBUTES                 (EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS)
#define SFP_SB_VIOLATION_SIGNAL_VAR_NAME  L"SecureBootAlert"

#define MSP_REBOOT_REASON_VAR_NAME        L"RebootReason"
#define MSP_REBOOT_REASON_LENGTH          8

// Reboot Reasons used for setting the front page icon
#define MSP_REBOOT_REASON_SETUP_KEY       "VOL+    " // Display VOL+ Icon
#define MSP_REBOOT_REASON_SETUP_BOOTFAIL  "BOOTFAIL" // Display Disk Icon
#define MSP_REBOOT_REASON_SETUP_SEC_FAIL  "BSecFail"
#define MSP_REBOOT_REASON_SETUP_OS        "OS      "
#define MSP_REBOOT_REASON_SETUP_NONE      "        " // Not a fail

#endif  // _MS_NV_BOOT_REASON_H_
