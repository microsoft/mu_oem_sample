/** @file
This BootGraphicsProviderLib is intended to abstract the source of the 
BMP files from the caller

This instance uses defined PCDs and RAW FFS files

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

#include <Pi/PiFirmwareFile.h>

#include <Library/PcdLib.h>
#include <Library/DebugLib.h>
#include <Library/BootGraphicsProviderLib.h>
#include <Library/DxeServicesLib.h>


/**
  Get the requested boot graphic

**/
EFI_STATUS
EFIAPI
GetBootGraphic (
  BOOT_GRAPHIC          Graphic,
  OUT UINTN             *ImageSize,
  OUT UINT8             **ImageData
  )
{
  EFI_GUID *g = NULL;

  switch(Graphic) {
    case BG_SYSTEM_LOGO:
      g = PcdGetPtr(PcdLogoFile);
      break;
    case BG_CRITICAL_OVER_TEMP:
      g = PcdGetPtr(PcdThermalFile); 
      break;
    case BG_CRITICAL_LOW_BATTERY:
      g = PcdGetPtr(PcdLowBatteryFile);
      break;
    default:
      DEBUG((DEBUG_ERROR, "Unsupported Boot Graphic Type 0x%X\n", Graphic));
      return EFI_UNSUPPORTED;
  }

  //
  // Get the specified image from FV.
  //
  return GetSectionFromAnyFv(g, EFI_SECTION_RAW, 0, (VOID **)ImageData, ImageSize);
}


UINT32
EFIAPI
GetBackgroundColor()
{
  return PcdGet32 (PcdPostBackgroundColor);
}
