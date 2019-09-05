/** @file PlatformKeyLib.h

  Copyright (C) Microsoft Corporation. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef __PLATFORM_KEY_LIB__
#define __PLATFORM_KEY_LIB__

/** TEST KEYS are not available when SHIP_MODE is defined  **/
#ifndef SHIP_MODE

//
// Test/Development Secure Boot PK Certificate 
//
extern CONST UINT8 *DevelopmentPlatformKeyCertificate;
extern CONST UINT32 DevelopmentSizeOfPlatformKeyCertificate;

#endif

/** PRODUCTION KEYS  **/

//
// Production Secure Boot PK Certificate 
//
extern CONST UINT8  *ProductionPlatformKeyCertificate;
extern CONST UINT32  ProductionSizeOfPlatformKeyCertificate;

#endif //__PLATFORM_KEY_LIB__