/** @file -- PlatformTpmPpProtocolPrivate.h

  Contains initialization and business logic for the platform TPM
  Physical Presence Protocol.

  Copyright (c) Microsoft Corporation.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#pragma once

/**
  Check to see whether either of the required Physical Presence variables
  is missing.
  If present, return current value.
  If missing, initialize it with the correct default value.

  @retval     EFI_SUCCESS  All required variables now exist.
  @retval     Others       Something went wrong and one or more variables is not
                           initialized.

**/
STATIC
EFI_STATUS
FindOrCreatePhysicalPresenceVariables (
  EFI_TCG2_PHYSICAL_PRESENCE_FLAGS  *PpiFlags,
  EFI_TCG2_PHYSICAL_PRESENCE        *TcgPpData
  );

/**
  Will save the results of a PP request so that the OS query the status.

  @param[in]  TcgPpData     A pointer to the current PP data.
  @param[in]  ResponseCode  The response/result to save.

  @retval     EFI_SUCCESS   Response successfully saved and should be readable by the OS.
  @retval     Others        CRITICAL ERROR! OS may get invalid or stale response data.

**/
STATIC
EFI_STATUS
LogPhysicalPresenceResult (
  IN  EFI_TCG2_PHYSICAL_PRESENCE  *TcgPpData,
  IN  UINT32                      ResponseCode
  );

/**
  Handles all the logic to prompt the user for confirmation of a
  TPM Physical Presence request.

  NOTE: Currently, we should NOT return from this function.
        The system will reset after performing the requested action.

  @param[in]  This

  @retval Any   Since we should never return from this function, any
                return value is an anomaly.

**/
STATIC
EFI_STATUS
EFIAPI
PpProcessUserConfirmation (
  IN TPM_PP_PROTOCOL  *This
  );
