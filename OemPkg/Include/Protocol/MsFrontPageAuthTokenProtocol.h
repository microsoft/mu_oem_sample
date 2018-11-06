/** @file  
  MsFrontPageAuthTokenProtocol is used to hold the auth token generated during the front page launch and will be accessed from across
  all frontpage applications to retrieve provider data from the settingsprovider. The authtoken should be disposed off on front page 
  exit and protocol unregistered.

  Copyright (c) 2015 - 2018, Microsoft Corporation.
  
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

#ifndef _FRONT_PAGE_AUTH_TOKEN_PROTOCOL_h
#define _FRONT_PAGE_AUTH_TOKEN_PROTOCOL_h

typedef struct _FRONT_PAGE_AUTH_TOKEN_PROTOCOL FRONT_PAGE_AUTH_TOKEN_PROTOCOL;

struct _FRONT_PAGE_AUTH_TOKEN_PROTOCOL{
  UINTN AuthToken;
};

extern EFI_GUID gMsFrontPageAuthTokenProtocolGuid;

#endif
