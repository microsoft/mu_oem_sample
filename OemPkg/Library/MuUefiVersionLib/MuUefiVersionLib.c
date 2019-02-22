/**@file Library to provide platform version information

Copyright (c) 2018, Microsoft Corporation

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

#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
#include <Library/PcdLib.h>
#include <Library/MuUefiVersionLib.h>


// Note the version below is platform specific
#define VERSION_YY_OFFSET	    0x2000U	// As defined through new versioning scheme design, epoch start from year 2000

#define VERSION_STR_TEMPLATE  L"%03X.%04X.%02X.%02X"
#define VERSION_STR_LENGTH    sizeof(L"GGG.YYYY.WW.RR")/sizeof(CHAR16) // Length of chars needed for version string,
                                                                       // Null-terminator included

#define DATE_STR_TEMPLATE     L"%02X/%02X/%04X"
#define DATE_STR_LENGTH       sizeof(L"MM/DD/YYYY")/sizeof(CHAR16)  // Length of chars needed for build date string,
                                                                    // Null-terminator included

typedef union {

	struct {
		UINT32 RevisionBCD  : 8;  // Revision number of this work week
		UINT32 WorkweekBCD  : 8;  // Work week number in BCD
		UINT32 YearBCD      : 8;  // Build year in BCD since year of 2000
		UINT32 GenerationBCD: 8;  // Gerenation number of this build
	} OemVer;

	UINT32 Raw;

} OEM_BUILD_ID_VER;

typedef union {

	struct {
		UINT32 DayBCD       : 8;  // Build day of month in BCD, range 1 - 31
		UINT32 MonthBCD     : 8;  // Build month in BCD, range 1 - 12
		UINT32 YearBCD      : 8;  // Build year in BCD since year of 2000
		UINT32 Unused       : 8;  // Reserved for extension
	} OemDate;

	UINT32 Raw;

} OEM_BUILD_DATE_VER;


/**
  Helper function to calculate and compare the target unicode string 
  length based on given template. The function will ASSERT() if there a
  mismatch is found between the result and TargetLength.

  @param TargetLength    Expected unicode string length, including Null-terminator.
  @param Template        The format of unicode string
  @param ...             Variable arguments based on the format string.

  @return TargetLength if there is no mismatch, otherwise ASSERT().

**/
STATIC
UINTN
VerifyStringLength (
  IN  UINTN         TargetLength,
  IN  CONST CHAR16  *Template,
  ...
)
{
  UINTN     Length;
  VA_LIST   Args;

  VA_START (Args, Template);
  Length = SPrintLength (Template, Args);
  VA_END(Args);

  // Compensate for Null-Terminator
  Length ++;
  ASSERT(Length == TargetLength);

  return Length;
}

/**
  Return Uefi version number defined by platform

  @retval  UINT32   UEFI firmware version
**/
UINT32
EFIAPI
GetUefiVersionNumber (
  VOID
)
{
  return PcdGet32(PcdUefiVersionNumber);
}

/**
  Return a Null-terminated Uefi version Ascii string defined by platform

  @param[out]       Buffer        The caller allocated buffer to hold the returned version
                                  Ascii string. May be NULL with a zero Length in order
                                  to determine the length buffer needed.
  @param[in, out]   Length        On input, the count of Ascii chars avaiable in Buffer.
                                  On output, the count of Ascii chars of data returned
                                  in Buffer, including Null-terminator.

  @retval EFI_SUCCESS             The function completed successfully.
  @retval EFI_BUFFER_TOO_SMALL    The Length is too small for the result.
  @retval EFI_INVALID_PARAMETER   Buffer is NULL.
  @retval EFI_INVALID_PARAMETER   Length is NULL.
  @retval EFI_INVALID_PARAMETER   The Length is not 0 and Buffer is NULL.
  @retval Others                  Other implementation specific errors.
**/
EFI_STATUS
EFIAPI
GetUefiVersionStringAscii (
      OUT CHAR8   *Buffer,            OPTIONAL
  IN  OUT UINTN   *Length
)
{
  EFI_STATUS        Status;
  CHAR16            UniBuffer[VERSION_STR_LENGTH];

  if ((Length == NULL) ||
      ((*Length != 0) && (Buffer == NULL))) {
    Status = EFI_INVALID_PARAMETER;
    goto Cleanup;
  }

  if (*Length < VERSION_STR_LENGTH) {
    Status = EFI_BUFFER_TOO_SMALL;
    goto Cleanup;
  }

  Status = GetUefiVersionStringUnicode(UniBuffer, Length);
  if (EFI_ERROR(Status)) {
    // Bail from here if Unicode function failed
    goto Cleanup;
  }

  UnicodeStrToAsciiStrS(UniBuffer, Buffer, *Length);
  Status = EFI_SUCCESS;

Cleanup:
  if (Length != NULL) {
    *Length = VERSION_STR_LENGTH;
  }
  return Status;
}

/**
  Return a Null-terminated Uefi version Unicode string defined by platform

  @param[out]       Buffer        The caller allocated buffer to hold the returned version
                                  Unicode string. May be NULL with a zero Length in order
                                  to determine the length buffer needed.
  @param[in, out]   Length        On input, the count of Unicode chars avaiable in Buffer.
                                  On output, the count of Unicode chars of data returned
                                  in Buffer, including Null-terminator.

  @retval EFI_SUCCESS             The function completed successfully.
  @retval EFI_BUFFER_TOO_SMALL    The Length is too small for the result.
  @retval EFI_INVALID_PARAMETER   Buffer is NULL.
  @retval EFI_INVALID_PARAMETER   Length is NULL.
  @retval EFI_INVALID_PARAMETER   The Length is not 0 and Buffer is NULL.
  @retval Others                  Other implementation specific errors.
**/
EFI_STATUS
EFIAPI
GetUefiVersionStringUnicode (
      OUT CHAR16  *Buffer,            OPTIONAL
  IN  OUT UINTN   *Length
)
{
  EFI_STATUS        Status;
  OEM_BUILD_ID_VER  Ver;

  if ((Length == NULL) ||
      ((*Length != 0) && (Buffer == NULL))) {
    Status = EFI_INVALID_PARAMETER;
    goto Cleanup;
  }

  if (*Length < VERSION_STR_LENGTH) {
    Status = EFI_BUFFER_TOO_SMALL;
    goto Cleanup;
  }

  Ver.Raw = GetUefiVersionNumber();

  // Check whether the template and predefined length are still a match
  *Length = VerifyStringLength( VERSION_STR_LENGTH,
                                VERSION_STR_TEMPLATE,
                                Ver.OemVer.GenerationBCD,
                                (Ver.OemVer.YearBCD | VERSION_YY_OFFSET),
                                Ver.OemVer.WorkweekBCD,
                                Ver.OemVer.RevisionBCD);
  if (*Length != VERSION_STR_LENGTH) {
    DEBUG((DEBUG_ERROR, "%a, String buffer length mismatch: have %d, expecting %d!", 
                        __FUNCTION__,
                        *Length,
                        VERSION_STR_LENGTH));
    Status = EFI_BAD_BUFFER_SIZE;
    goto Cleanup;
  }

  UnicodeSPrint(Buffer,
                VERSION_STR_LENGTH * sizeof(CHAR16),
                VERSION_STR_TEMPLATE,
                Ver.OemVer.GenerationBCD,
                (Ver.OemVer.YearBCD | VERSION_YY_OFFSET),
                Ver.OemVer.WorkweekBCD,
                Ver.OemVer.RevisionBCD
                );
  Status = EFI_SUCCESS;

Cleanup:
  if (Length != NULL) {
    *Length = VERSION_STR_LENGTH;
  }
  return Status;
}

/**
  Return a Null-terminated Uefi build date Ascii string defined by platform

  @param[out]       Buffer        The caller allocated buffer to hold the returned build
                                  date Ascii string. May be NULL with a zero Length in
                                  order to determine the length buffer needed.
  @param[in, out]   Length        On input, the count of Ascii chars avaiable in Buffer.
                                  On output, the count of Ascii chars of data returned
                                  in Buffer, including Null-terminator.

  @retval EFI_SUCCESS             The function completed successfully.
  @retval EFI_BUFFER_TOO_SMALL    The Length is too small for the result.
  @retval EFI_INVALID_PARAMETER   Buffer is NULL.
  @retval EFI_INVALID_PARAMETER   Length is NULL.
  @retval EFI_INVALID_PARAMETER   The Length is not 0 and Buffer is NULL.
  @retval Others                  Other implementation specific errors.
**/
EFI_STATUS
EFIAPI
GetBuildDateStringAscii (
      OUT CHAR8   *Buffer,            OPTIONAL
  IN  OUT UINTN   *Length
)
{
  EFI_STATUS          Status;
  CHAR16              UniBuffer[DATE_STR_LENGTH];

  if ((Length == NULL) ||
      ((*Length != 0) && (Buffer == NULL))) {
    Status = EFI_INVALID_PARAMETER;
    goto Cleanup;
  }

  if (*Length < DATE_STR_LENGTH) {
    Status = EFI_BUFFER_TOO_SMALL;
    goto Cleanup;
  }

  Status = GetBuildDateStringUnicode(UniBuffer, Length);
  if (EFI_ERROR(Status)) {
    // Bail from here if Unicode function failed
    goto Cleanup;
  }

  UnicodeStrToAsciiStrS(UniBuffer, Buffer, *Length);
  Status = EFI_SUCCESS;

Cleanup:
  if (Length != NULL) {
    *Length = DATE_STR_LENGTH;
  }
  return Status;
}

/**
  Return a Null-terminated Uefi build date Unicode defined by platform

  @param[out]       Buffer        The caller allocated buffer to hold the returned build
                                  date Unicode string. May be NULL with a zero Length in
                                  order to determine the length buffer needed.
  @param[in, out]   Length        On input, the count of Unicode chars avaiable in Buffer.
                                  On output, the count of Unicode chars of data returned 
                                  in Buffer, including Null-terminator.

  @retval EFI_SUCCESS             The function completed successfully.
  @retval EFI_BUFFER_TOO_SMALL    The Length is too small for the result.
  @retval EFI_INVALID_PARAMETER   Buffer is NULL.
  @retval EFI_INVALID_PARAMETER   Length is NULL.
  @retval EFI_INVALID_PARAMETER   The Length is not 0 and Buffer is NULL.
  @retval Others                  Other implementation specific errors.
**/
EFI_STATUS
EFIAPI
GetBuildDateStringUnicode (
      OUT CHAR16  *Buffer,            OPTIONAL
  IN  OUT UINTN   *Length
)
{
  EFI_STATUS          Status;
  OEM_BUILD_DATE_VER  BuildDate;

  if ((Length == NULL) ||
      ((*Length != 0) && (Buffer == NULL))) {
    Status = EFI_INVALID_PARAMETER;
    goto Cleanup;
  }

  if (*Length < DATE_STR_LENGTH) {
    Status = EFI_BUFFER_TOO_SMALL;
    goto Cleanup;
  }

  BuildDate.Raw = PcdGet32(PcdUefiBuildDate);

  // Check whether the template and predefined length are still a match
  *Length = VerifyStringLength( DATE_STR_LENGTH,
                                DATE_STR_TEMPLATE,
                                BuildDate.OemDate.MonthBCD,
                                BuildDate.OemDate.DayBCD,
                                (BuildDate.OemDate.YearBCD | VERSION_YY_OFFSET));
  if (*Length != DATE_STR_LENGTH) {
    DEBUG((DEBUG_ERROR, "%a, String buffer length mismatch: have %d, expecting %d!", 
                        __FUNCTION__,
                        *Length,
                        DATE_STR_LENGTH));
    Status = EFI_BAD_BUFFER_SIZE;
    goto Cleanup;
  }

  UnicodeSPrint(Buffer,
                DATE_STR_LENGTH * sizeof(CHAR16),
                DATE_STR_TEMPLATE,
                BuildDate.OemDate.MonthBCD,
                BuildDate.OemDate.DayBCD,
                (BuildDate.OemDate.YearBCD | VERSION_YY_OFFSET)
                );
  Status = EFI_SUCCESS;

Cleanup:
  if (Length != NULL) {
    *Length = DATE_STR_LENGTH;
  }
  return Status;
}

