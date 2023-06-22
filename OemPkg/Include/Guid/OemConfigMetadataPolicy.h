/** @file
  This file defines GUIDs and data structures for the OEM Config Metadata Policy.

  Copyright (c) Microsoft Corporation.

**/

#include <Uefi.h>

#ifndef  OEM_CONFIG_METADATA_POLICY_H_
#define  OEM_CONFIG_METADATA_POLICY_H_

#define OEM_CONFIG_METADATA_POLICY_GUID  \
  { \
    0X44E9778F, 0X3DAF, 0X46BA, { 0XB1, 0X86, 0X78, 0X4D, 0X0B, 0X05, 0X50, 0X72 } \
  }

#define OEM_CONFIG_METADATA_POLICY_SIZE  sizeof(OEM_CONFIG_METADATA_POLICY)

#define GENERIC_PROFILE_FLAVOR_NAME  "GN"

  #pragma pack(1)

typedef struct {
  UINT32    ActiveProfileIndex;
  CHAR8     ActiveProfileFlavorName[PROFILE_FLAVOR_NAME_LENGTH];
} OEM_CONFIG_METADATA_POLICY;

  #pragma pack()

extern EFI_GUID  gOemConfigMetadataPolicyGuid;

#endif // OEM_CONFIG_METADATA_POLICY_H_
