/** @file PlatformKeyLibNull.c

  Copyright (C) Microsoft Corporation. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

/** TEST KEYS are not available when SHIP_MODE is defined  **/
#ifndef SHIP_MODE

//
// Test/Development Secure Boot PK Certificate 
//
CONST UINT8  kaDevelopmentPlatformKeyCertificate[1] = {0};
CONST UINT8  *DevelopmentPlatformKeyCertificate = kaDevelopmentPlatformKeyCertificate;
CONST UINT32 DevelopmentSizeOfPlatformKeyCertificate = 0;


#endif

/** PRODUCTION KEYS  **/

//
// Production Secure Boot PK Certificate 
//
CONST UINT8   kaProductionPlatformKeyCertificate[1] = {0};
CONST UINT8   *ProductionPlatformKeyCertificate = kaProductionPlatformKeyCertificate;
CONST UINT32  ProductionSizeOfPlatformKeyCertificate=0;
