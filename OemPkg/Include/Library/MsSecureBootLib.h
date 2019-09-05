/*++ @file MsSecureBootLib.h

  Copyright (C) Microsoft Corporation. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent

Module Name:

  MsSecureBootLib.h

Abstract:

  Secure Boot functions.

Environments:

  Driver Execution Environment (DXE)

--*/

#ifndef __MS_SECURE_BOOT_LIB_INC__
#define __MS_SECURE_BOOT_LIB_INC__


enum {
    MS_SB_CONFIG_MS_ONLY = 0,   // Starts at 0 so it can be used as an index.
    MS_SB_CONFIG_MS_3P,
    MS_SB_CONFIG_NONE,

    // ALWAYS LAST OPTION!!
    MS_SB_CONFIG_COUNT
};


/**
    This fucntion will delete the secure boot keys, thus
    disabling it.


  @return EFI_SUCCESS or underlying failure code.

**/
EFI_STATUS
EFIAPI
DeleteSecureBootVariables();


/**
  NOTE: Copied from SecureBootConfigImpl.c, then modified.

  Create a time based data payload by concatenating the EFI_VARIABLE_AUTHENTICATION_2
  descriptor with the input data. NO authentication is required in this function.

  @param[in, out]   DataSize       On input, the size of Data buffer in bytes.
                                   On output, the size of data returned in Data
                                   buffer in bytes.
  @param[in, out]   Data           On input, Pointer to data buffer to be wrapped or
                                   pointer to NULL to wrap an empty payload.
                                   On output, Pointer to the new payload date buffer allocated from pool,
                                   it's caller's responsibility to free the memory when finish using it.
  @param[in]        Time           [Optional] If provided, will be used as the timestamp for the payload.
                                   If NULL, a new timestamp will be generated using GetTime().

  @retval EFI_SUCCESS              Create time based payload successfully.
  @retval EFI_OUT_OF_RESOURCES     There are not enough memory resourses to create time based payload.
  @retval EFI_INVALID_PARAMETER    The parameter is invalid.
  @retval Others                   Unexpected error happens.

**/
STATIC
EFI_STATUS
CreateTimeBasedPayload (
  IN OUT UINTN            *DataSize,
  IN OUT UINT8            **Data,
  IN     EFI_TIME         *Time OPTIONAL
  );


/**
  Signals the SMM Variable services that an "authorized" PK
  modification is about to occur. Before ReadyToBoot this
  *should* allow an update to the PK without validating the
  full signature.

  @param[in]  State   TRUE = PK update is authorized. Set indication tokens appropriately.
                      FALSE = PK update is not authorized. Clear all indication tokens.

  @retval     EFI_SUCCESS             State has been successfully updated.
  @retval     EFI_INVALID_PARAMETER   State is something other than TRUE or FALSE.
  @retval     EFI_SECURITY_VIOLATION  Attempting to an invalid state at an invalid time (eg. post-ReadyToBoot).
  @retval     Others                  Error returned from SetVariable.

**/
STATIC
EFI_STATUS
SetAuthorizedPkUpdateState (
  IN  BOOLEAN   State
  );


/**
  Helper function to quickly determine whether SecureBoot is enabled.

  @retval     TRUE    SecureBoot is verifiably enabled.
  @retval     FALSE   SecureBoot is either disabled or an error prevented checking.

**/
BOOLEAN
IsSecureBootEnable (
  VOID
  );


/**
  Returns the current config of the SecureBoot variables, if it can be determined.

  @retval     UINTN   Will return an MS_SB_CONFIG token or -1 if the config cannot be determined.

**/
UINTN
GetCurrentSecureBootConfig (
  VOID
  );


/**
  Similar to DeleteSecureBootVariables, this function is used to unilaterally
  force the state of all 4 SB variables. Use build-in, hardcoded default vars.

  NOTE: The UseThirdParty parameter can be used to set either strict MS or
        MS+3rdParty keys.

  @param[in]  UseThirdParty  Flag to indicate whether to use 3rd party keys or
                             strict MS-only keys.

  @retval     EFI_SUCCESS               SecureBoot keys are now set to defaults.
  @retval     EFI_ABORTED               SecureBoot keys are not empty. Please delete keys first
                                        or follow standard methods of altering keys (ie. use the signing system).
  @retval     EFI_SECURITY_VIOLATION    Failed to create the PK.
  @retval     Others                    Something failed in one of the subfunctions.

**/
EFI_STATUS
SetDefaultSecureBootVariables (
  IN  BOOLEAN    UseThirdParty
  );

#endif //__MS_SECURE_BOOT_LIB_INC__
