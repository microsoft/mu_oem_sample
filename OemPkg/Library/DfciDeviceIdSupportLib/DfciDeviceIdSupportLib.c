/** @file DfciDeviceIdSupportLib.c

This library provides access to platform data that becomes the DFCI DeviceId.

Copyright (C) Microsoft Corporation. All rights reserved.
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <PiDxe.h>
#include <Library/DfciDeviceIdSupportLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>

#include <Uefi/UefiInternalFormRepresentation.h>
#include <Protocol/Smbios.h>

#define ID_NOT_FOUND      "Not Found"

// Note: This protocol will guarantee to be met by the Depex and located at the
// constructor of this library, thus no null-pointer check in library code flow.
EFI_SMBIOS_PROTOCOL       *mSmbiosProtocol;

/**

  Acquire the string associated with the Index from smbios structure and return it.
  The caller is responsible for free the string buffer.

  @param    OptionalStrStart  The start position to search the string
  @param    Index             The index of the string to extract
  @param    String            The string that is extracted
  @param    Size              Optional pointer to hold size of returned string

  @retval   EFI_SUCCESS       The function returns EFI_SUCCESS always.

**/
EFI_STATUS
GetOptionalStringByIndex (
  IN      CHAR8                   *OptionalStrStart,
  IN      UINT8                   Index,
  OUT     CHAR8                   **String,
  OUT     UINTN                   *Size   OPTIONAL
  )
{
  UINTN          StrSize;

  if (Index == 0) {
    *String = AllocateZeroPool (sizeof (CHAR16));
    return EFI_SUCCESS;
  }

  StrSize = 0;
  do {
    Index--;
    OptionalStrStart += StrSize;
    StrSize           = AsciiStrSize (OptionalStrStart);
  } while (OptionalStrStart[StrSize] != 0 && Index != 0);

  if ((Index != 0) || (StrSize == 1)) {
    //
    // Meet the end of strings set but Index is non-zero, or
    // Find an empty string
    //
    StrSize = sizeof(ID_NOT_FOUND);
    *String = AllocatePool (StrSize);
    CopyMem (*String, ID_NOT_FOUND, StrSize);
  } else {
    *String = AllocatePool (StrSize);
    CopyMem (*String, OptionalStrStart, StrSize);
  }

  if (Size != NULL) {
    *Size = StrSize;
  }

  return EFI_SUCCESS;
}

/**
Gets the serial number for this device.

@para[out] SerialNumber - UINTN value of serial number

@return EFI_SUCCESS - SerialNumber has been updated to equal the serial number of the device
@return EFI_ERROR   - Error getting number

**/
EFI_STATUS
EFIAPI
DfciIdSupportV1GetSerialNumber(
  OUT UINTN*  SerialNumber
  ) {
  EFI_STATUS                Status;
  CHAR8                     *NewString;
  EFI_SMBIOS_HANDLE         SmbiosHandle;
  EFI_SMBIOS_TABLE_HEADER   *Record;
  SMBIOS_TYPE               Type;
  SMBIOS_TABLE_TYPE1        *Type1Record;

  SmbiosHandle = SMBIOS_HANDLE_PI_RESERVED; // Reset handle
  Type = SMBIOS_TYPE_SYSTEM_INFORMATION;    // Smbios type1
  Status = mSmbiosProtocol->GetNext(mSmbiosProtocol, &SmbiosHandle, &Type, &Record, NULL);
  if (!EFI_ERROR(Status))
  {
    Type1Record = (SMBIOS_TABLE_TYPE1 *) Record;
  }

  Status = GetOptionalStringByIndex ((CHAR8*)((UINT8*)Type1Record + Type1Record->Hdr.Length), Type1Record->SerialNumber, &NewString, NULL);
  if (!EFI_ERROR(Status)) {
    CopyMem (SerialNumber, NewString, sizeof(UINTN));
    FreePool (NewString);
  }
   return Status;
}

/**
 * Get the Manufacturer Name
 *
 * @param Manufacturer
 * @param ManufacturerSize
 *
 * It is the callers responsibility to free the buffer returned.
 *
 * @return EFI_STATUS EFIAPI
 */
EFI_STATUS
EFIAPI
DfciIdSupportGetManufacturer (
    CHAR8   **Manufacturer,
    UINTN    *ManufacturerSize   OPTIONAL
  ) {

  EFI_STATUS                Status;
  EFI_SMBIOS_HANDLE         SmbiosHandle;
  EFI_SMBIOS_TABLE_HEADER   *Record;
  SMBIOS_TYPE               Type;
  SMBIOS_TABLE_TYPE1        *Type1Record;

  if (Manufacturer == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  SmbiosHandle = SMBIOS_HANDLE_PI_RESERVED; // Reset handle
  Type = SMBIOS_TYPE_SYSTEM_INFORMATION;    // Smbios type1
  Status = mSmbiosProtocol->GetNext(mSmbiosProtocol, &SmbiosHandle, &Type, &Record, NULL);
  if (!EFI_ERROR(Status))
  {
    Type1Record = (SMBIOS_TABLE_TYPE1 *) Record;
  }

  Status = GetOptionalStringByIndex ((CHAR8*)((UINT8*)Type1Record + Type1Record->Hdr.Length), Type1Record->Manufacturer, Manufacturer, ManufacturerSize);
  return Status;
}

/**
 * Get the ProductName
 *
 * @param ProductName
 * @param ProductNameSize
 *
 * It is the callers responsibility to free the buffer returned.
 *
 * @return EFI_STATUS EFIAPI
 */
EFI_STATUS
EFIAPI
DfciIdSupportGetProductName (
    CHAR8   **ProductName,
    UINTN    *ProductNameSize  OPTIONAL
  ) {

  EFI_STATUS                Status;
  EFI_SMBIOS_HANDLE         SmbiosHandle;
  EFI_SMBIOS_TABLE_HEADER   *Record;
  SMBIOS_TYPE               Type;
  SMBIOS_TABLE_TYPE1        *Type1Record;

  if (ProductName == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  SmbiosHandle = SMBIOS_HANDLE_PI_RESERVED; // Reset handle
  Type = SMBIOS_TYPE_SYSTEM_INFORMATION;    // Smbios type1
  Status = mSmbiosProtocol->GetNext(mSmbiosProtocol, &SmbiosHandle, &Type, &Record, NULL);
  if (!EFI_ERROR(Status))
  {
    Type1Record = (SMBIOS_TABLE_TYPE1 *) Record;
  }

  Status = GetOptionalStringByIndex ((CHAR8*)((UINT8*)Type1Record + Type1Record->Hdr.Length), Type1Record->ProductName, ProductName, ProductNameSize);
  return Status;
}

/**
 * Get the SerialNumber
 *
 * @param SerialNumber
 * @param SerialNumberSize
 *
 * It is the callers responsibility to free the buffer returned.
 *
 * @return EFI_STATUS EFIAPI
 */
EFI_STATUS
EFIAPI
DfciIdSupportGetSerialNumber (
    CHAR8   **SerialNumber,
    UINTN    *SerialNumberSize  OPTIONAL
  ) {

  EFI_STATUS                Status;
  EFI_SMBIOS_HANDLE         SmbiosHandle;
  EFI_SMBIOS_TABLE_HEADER   *Record;
  SMBIOS_TYPE               Type;
  SMBIOS_TABLE_TYPE3        *Type3Record;

  if (SerialNumber == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  SmbiosHandle = SMBIOS_HANDLE_PI_RESERVED; // Reset handle
  Type = SMBIOS_TYPE_SYSTEM_ENCLOSURE;    // Smbios type3
  Status = mSmbiosProtocol->GetNext(mSmbiosProtocol, &SmbiosHandle, &Type, &Record, NULL);
  if (!EFI_ERROR(Status))
  {
    Type3Record = (SMBIOS_TABLE_TYPE3 *) Record;
  }

  Status = GetOptionalStringByIndex ((CHAR8*)((UINT8*)Type3Record + Type3Record->Hdr.Length), Type3Record->SerialNumber, SerialNumber, SerialNumberSize);
  return Status;
}

/**
  Constructor for DfciIdSupportLib.

  @param  ImageHandle   ImageHandle of the loaded driver.
  @param  SystemTable   Pointer to the EFI System Table.

  @retval EFI_SUCCESS   The handlers were registered successfully.
**/
EFI_STATUS
EFIAPI
DfciIdSupportConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                Status;

  Status = gBS->LocateProtocol(&gEfiSmbiosProtocolGuid, NULL, (VOID**)&mSmbiosProtocol);
  if (EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "Could not locate SMBIOS protocol.  %r\n", Status));
  }

  return Status;
}
