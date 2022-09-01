/** @file

  Helper library to support the query of the in-effect MFCI policy and
  translate raw policy to OEM defined operation modes.

  Copyright (c) Microsoft Corporation
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef OEM_MFCI_LIB_H_
#define OEM_MFCI_LIB_H_

#define OEM_UEFI_CUSTOMER_MODE       0
#define OEM_UEFI_MANUFACTURING_MODE  1

typedef UINT64 OEM_UEFI_OPERATION_MODE;

/**
 * Inspect whether the current operation mode is categorized as manufacturing mode.
 *
 * @param[out] OperationMode      64 bit value from MFCI framework, indicating the current operation mode
 *
 * @return OEM_UEFI_CUSTOMER_MODE         Current mode is customer mode.
 * @return OEM_UEFI_MANUFACTURING_MODE    Current operation mode is manufacturing mode.
 */
OEM_UEFI_OPERATION_MODE
EFIAPI
GetMfciSystemOperationMode (
  VOID
  );

#endif //OEM_MFCI_LIB_H_
