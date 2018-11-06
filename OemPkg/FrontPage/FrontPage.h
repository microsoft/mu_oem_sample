/** @file
  FrontPage routines to handle the callbacks and browser calls

Copyright (c) 2004 - 2012, Intel Corporation. All rights reserved.<BR>
Copyright (c) 2015 - 2018, Microsoft Corporation. All rights reserved.<BR>

This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _FRONT_PAGE_H_
#define _FRONT_PAGE_H_

#include <Protocol/FormBrowser2.h>
#include <Protocol/HiiConfigAccess.h>
#include "FrontPageVfr.h"  // all shared VFR / C constants here.  
#include <DfciSystemSettingTypes.h>
#include <Protocol/DfciSettingAccess.h>
#include <Library/HiiLib.h>
#include <Protocol/MsFrontPageAuthTokenProtocol.h>
#include <Protocol/DfciAuthentication.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/HiiConfigRouting.h>

//
// These are the VFR compiler generated data representing our VFR data.
//
extern UINT8    FrontPageVfrBin[];

extern UINTN    mCallbackKey;

#define FRONT_PAGE_CALLBACK_DATA_SIGNATURE  SIGNATURE_32 ('F', 'P', 'C', 'B')

typedef struct {
  UINTN                           Signature;

  //
  // HII relative handles
  //
  EFI_HII_HANDLE                  HiiHandle;
  EFI_HANDLE                      DriverHandle;
  EFI_STRING_ID                   *LanguageToken;

  //
  // Produced protocols
  //
  EFI_HII_CONFIG_ACCESS_PROTOCOL  ConfigAccess;
} FRONT_PAGE_CALLBACK_DATA;

/**
  Initialize HII information for the FrontPage


  @param InitializeHiiData    TRUE if HII elements need to be initialized.

  @retval  EFI_SUCCESS        The operation is successful.
  @retval  EFI_DEVICE_ERROR   If the dynamic opcode creation failed.

**/
EFI_STATUS
InitializeFrontPage (
  IN BOOLEAN    InitializeHiiData
  );


#endif // _FRONT_PAGE_H_

