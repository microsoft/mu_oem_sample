
/** @file FrontpageButtonsVolumeUp.c

  This module installs the MsButtonServicesProtocol and reports that Vol+ and power are being pressed.
  This will cause FrontPage to be selected on each boot.
  Not to be used in production.

  Copyright (c) 2010, Microsoft Corporation.
  
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

#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <Protocol/ButtonServices.h>


/*
Say volume button is pressed because we wan to go to frontpage.
*/
EFI_STATUS
EFIAPI
PreBootVolumeUpButtonThenPowerButtonCheck(
  IN  MS_BUTTON_SERVICES_PROTOCOL *This,
  OUT BOOLEAN                     *PreBootVolumeUpButtonThenPowerButton // TRUE if button combo set else FALSE
)
{
  DEBUG((DEBUG_ERROR, "%a \n", __FUNCTION__));
  *PreBootVolumeUpButtonThenPowerButton = TRUE;
  return EFI_SUCCESS;
}


/*
Say no because we don't want alt boot.
*/
EFI_STATUS
EFIAPI
PreBootVolumeDownButtonThenPowerButtonCheck(
  IN  MS_BUTTON_SERVICES_PROTOCOL *This,
  OUT BOOLEAN *PreBootVolumeDownButtonThenPowerButton // TRUE if button combo set else FALSE
)
{
  DEBUG((DEBUG_ERROR, "%a \n", __FUNCTION__));
  *PreBootVolumeDownButtonThenPowerButton = FALSE; // default to not pressed
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
PreBootClearVolumeButtonState(
  MS_BUTTON_SERVICES_PROTOCOL *This
) 
{
  DEBUG((DEBUG_ERROR, "%a \n", __FUNCTION__));
  return EFI_SUCCESS;
}

/**
 Init routine to install protocol and init anything related to buttons

 **/
EFI_STATUS
EFIAPI
ButtonsInit(
  IN EFI_HANDLE                   ImageHandle,
  IN EFI_SYSTEM_TABLE             *SystemTable
)
{
  MS_BUTTON_SERVICES_PROTOCOL* Protocol = NULL;
  EFI_STATUS Status = EFI_SUCCESS;
  DEBUG((DEBUG_ERROR, "%a \n", __FUNCTION__));

  Protocol = AllocateZeroPool(sizeof(MS_BUTTON_SERVICES_PROTOCOL));
  if (Protocol == NULL)
  {
    DEBUG((DEBUG_ERROR, "Failed to allocate memory for button service protocol.\n"));
    return EFI_OUT_OF_RESOURCES;
  }

  Protocol->PreBootVolumeDownButtonThenPowerButtonCheck = PreBootVolumeDownButtonThenPowerButtonCheck;
  Protocol->PreBootVolumeUpButtonThenPowerButtonCheck = PreBootVolumeUpButtonThenPowerButtonCheck;
  Protocol->PreBootClearVolumeButtonState = PreBootClearVolumeButtonState;

  // Install the protocol
  Status = gBS->InstallMultipleProtocolInterfaces(&ImageHandle,
    &gMsButtonServicesProtocolGuid,
    Protocol,
    NULL
    );

  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "Button Services Protocol Publisher: install protocol error, Status = %r.\n", Status));
    FreePool(Protocol);
    return Status;
  }

  DEBUG((DEBUG_INFO, "Button Services Protocol Installed!\n"));
  return Status;
}
