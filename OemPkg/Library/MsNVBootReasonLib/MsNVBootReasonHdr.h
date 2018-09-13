/** @file
Header file to support the Reboot Reason non volatile varialbles

Copyright (c) 2015 - 2018, Microsoft Corporation

All rights reserved.
Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

**/

#ifndef _MS_NV_BOOT_REASON_HDR_H_
#define _MS_NV_BOOT_REASON_HDR_H_

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

#endif  // _MS_NV_BOOT_REASON_HDR_H_
