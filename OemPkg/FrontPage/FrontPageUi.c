/** @file
  User interaction functions for the FrontPage.

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

#include "FrontPage.h"
#include "FrontPageUi.h"

#include <PiDxe.h>          // This has to be here so Protocol/FirmwareVolume2.h doesn't puke errors.

#include <Guid/GlobalVariable.h>
#include <Guid/ImageAuthentication.h>
#include <Guid/SmmVariableCommon.h>
#include <Guid/MdeModuleHii.h>

#include <Protocol/OnScreenKeyboard.h>
#include <Protocol/SimpleWindowManager.h>
#include <Protocol/FirmwareVolume2.h>
#include <Protocol/SmmCommunication.h>
#include <Protocol/SmmVariable.h>

#include <Library/BaseMemoryLib.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseLib.h>
#include <Library/HiiLib.h>
#include <Library/PrintLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootManagerLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/MsColorTableLib.h>
#include <Library/SwmDialogsLib.h>

/**
  This function processes the results of changes in configuration.

  @param This            Points to the EFI_HII_CONFIG_ACCESS_PROTOCOL.
  @param Action          Specifies the type of action taken by the browser.
  @param QuestionId      A unique value which is sent to the original exporting driver
                         so that it can identify the type of data to expect.
  @param Type            The type of value for the question.
  @param Value           A pointer to the data being sent to the original exporting driver.
  @param ActionRequest   On return, points to the action requested by the callback function.

  @retval  EFI_SUCCESS           The callback successfully handled the action.
  @retval  EFI_OUT_OF_RESOURCES  Not enough storage is available to hold the variable and its data.
  @retval  EFI_DEVICE_ERROR      The variable could not be saved.
  @retval  EFI_UNSUPPORTED       The specified Action is not supported by the callback.

**/
EFI_STATUS
EFIAPI
UiCallback (
           IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
           IN  EFI_BROWSER_ACTION                     Action,
           IN  EFI_QUESTION_ID                        QuestionId,
           IN  UINT8                                  Type,
           IN  EFI_IFR_TYPE_VALUE                     *Value,
           OUT EFI_BROWSER_ACTION_REQUEST             *ActionRequest
           )
{
    EFI_STATUS    Status = EFI_SUCCESS;

    DEBUG ((DEBUG_INFO, "FrontPage:UiCallback() - Question ID=0x%08x Type=0x%04x Action=0x%04x ShortValue=0x%02x\n", QuestionId, Type, Action, *(UINT8*)Value));

    //
    // Sanitize input values.
    if (Value == NULL || ActionRequest == NULL)
    {
        DEBUG ((DEBUG_INFO, "FrontPage:UiCallback - Bailing from invalid input.\n"));
        return EFI_INVALID_PARAMETER;
    }

    //
    // Filter responses.
    // NOTE: For now, let's only consider elements that have CHANGED.
    if (Action != EFI_BROWSER_ACTION_CHANGED)
    {
        DEBUG ((DEBUG_INFO, "FrontPage:UiCallback - Bailing from unimportant input.\n"));
        return EFI_UNSUPPORTED;
    }

    //
    // Set a default action request.
    *ActionRequest = EFI_BROWSER_ACTION_REQUEST_NONE;

    //
    // Handle the specific callback.
    // We'll record the callback event as mCallbackKey so that other processes can make decisions
    // on how we exited the run loop (if that occurs).
    mCallbackKey = QuestionId;
    switch (mCallbackKey)
    {
    case FRONT_PAGE_ACTION_EXIT_FRONTPAGE:
        *ActionRequest = EFI_BROWSER_ACTION_REQUEST_EXIT;
        break;

    default:
        DEBUG ((DEBUG_INFO, "FrontPage:UiCallback - Unknown event passed.\n"));
        Status = EFI_UNSUPPORTED;
        mCallbackKey = 0;
        break;
    }

    return Status;
}
