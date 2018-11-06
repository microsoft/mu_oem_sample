/**
 * Front Page vfr and c file common definitions

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

//
// THIS FILE MUST ONLY CONTAIN DEFINTIONS THAT CAN BE INTERPRETED 
// BY BOTH THE VFR AND C COMPILERS.
//

#ifndef FRONT_PAGE_VFR_H
#define FRONT_PAGE_VFR_H


#define FRONT_PAGE_CLASS                                0x0000
#define FRONT_PAGE_SUBCLASS                             0x0002

//
// This is the VFR compiler generated header file which defines the
// string identifiers.
//
#define PRINTABLE_LANGUAGE_NAME_STRING_ID               0x0001

// Front Page exposes the following forms
//
// NOTE: Form ID order and values must align with VFR code.
//
#define FRONT_PAGE_FORM_ID_NONE                         0x0000
#define FRONT_PAGE_FORM_ID_PCINFO                       0x0001
#define FRONT_PAGE_FORM_ID_BOOTMENU                     0x0002
#define FRONT_PAGE_FORM_ID_EXIT                         0x0003

// Front Page triggers the following actions
//
// NOTE: Form ID order and values must align with VFR code.
//
#define FRONT_PAGE_ACTION_DEFAULTS                      0x1000
#define FRONT_PAGE_ACTION_EXIT_FRONTPAGE                0x1001

#define LABEL_PCINFO_FW_VERSION_TAG_START               0x2000
#define LABEL_PCINFO_FW_VERSION_TAG_END                 0x2001

// Grid class Start delimeter (GUID opcode).
//
#define GRID_START_OPCODE_GUID                                             \
  {                                                                                \
    0xc0b6e247, 0xe140, 0x4b4d, { 0xa6, 0x4, 0xc3, 0xae, 0x1f, 0xa6, 0xcc, 0x12 }  \
  }

// Grid class End delimeter (GUID opcode).
//  
#define GRID_END_OPCODE_GUID                                               \
  {                                                                                \
    0x30879de9, 0x7e69, 0x4f1b, { 0xb5, 0xa5, 0xda, 0x15, 0xbf, 0x6, 0x25, 0xce }  \
  }

// Front Page Formset Guid
// {7F98EFE9-50AA-4598-B7C1-CB72E1CC5224}
#define FRONT_PAGE_CONFIG_FORMSET_GUID \
  { \
    0x7f98efe9, 0x50aa, 0x4598, { 0xb7, 0xc1, 0xcb, 0x72, 0xe1, 0xcc, 0x52, 0x24 } \
  }

#endif  // FRONT_PAGE_VFR_H
