/** @file  
  MsFrontPageAuthTokenProtocol is used to hold the auth token generated during the front page launch and will be accessed from across
  all frontpage applications to retrieve provider data from the settingsprovider. The authtoken should be disposed off on front page 
  exit and protocol unregistered.

  Copyright (C) Microsoft Corporation. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef _FRONT_PAGE_AUTH_TOKEN_PROTOCOL_h
#define _FRONT_PAGE_AUTH_TOKEN_PROTOCOL_h

typedef struct _FRONT_PAGE_AUTH_TOKEN_PROTOCOL FRONT_PAGE_AUTH_TOKEN_PROTOCOL;

struct _FRONT_PAGE_AUTH_TOKEN_PROTOCOL{
  UINTN AuthToken;
};

extern EFI_GUID gMsFrontPageAuthTokenProtocolGuid;

#endif
