# FDF Include File in Detail

This readme aims to describe FrontpageDsc.inc in detail, providing details and justification for
the libraries and components included for Frontpage.

## Preliminaries

### What is an FDF File

The .fdf filetype is an EDK2 Platform Description file format. The FDF file is used to describe the content and
layout of binary images. It is used in conjunction with an EDK2 DSC file to generate bootable images,
option ROM images, and update capsules for bootable images that comply with the UEFI specifications. To
learn more about FDF files, look at the
[specification](https://edk2-docs.gitbook.io/edk-ii-fdf-specification/)
on the tianocore-docs github.

### What is an INF File

The .inf filetype is an EDK2 Platform Description file format. The INF file describes properties of a module,
how it is coded, what it provides, what it depends on, architecture specific items, features, etc. regarding
the module. To learn more about INF files, look at the
[specification](https://edk2-docs.gitbook.io/edk-ii-inf-specification/)
on the tianocore-docs github.

### How do I Include an FDF File

This file will be included in the DSC file for the platform being built. The syntax for including a file
is:

    !include path/to/fdf/inc/file

### What is DFCI

From the [feature document](https://microsoft.github.io/mu/dyn/mu_plus/DfciPkg/Docs/Dfci_Feature/):
*The Device Firmware Configuration Interface (DFCI) brings new levels of security and usability to
PC configuration management. It is a new feature of UEFI that enables secure programmatic configuration
of hardware settings that are typically configured within a BIOS menu.*

## FDF Breakdown

This first line is a comment simply alerting the dev to include this file at some point under the
[FV.FvMain] section of the platform FDF file.

    # Continuation of [FV.FvMain]

Finds and takes ownership of gEfiGraphicsOutputProtocolGuid so RenderingEngine can use it by uninstalling
it on its current handle and reinstalling it on a new one.

    INF MsGraphicsPkg/GopOverrideDxe/GopOverrideDxe.inf

AuthManager provides authentication for [DFCI](#what-is-dfci).

    INF DfciPkg/IdentityAndAuthManager/IdentityAndAuthManagerDxe.inf

The main component of [DFCI](#what-is-dfci) which manages and enforces all DFCI settings

    INF DfciPkg/SettingsManager/SettingsManagerDxe.inf

Installs protocol to share the UI theme.

    INF MsGraphicsPkg/MsUiTheme/Dxe/MsUiThemeProtocol.inf

Produces EfiGraphicsOutputProtocol to draw graphics to the screen

    INF MsGraphicsPkg/RenderingEngineDxe/RenderingEngineDxe.inf

Produces gEdkiiFormDisplayEngineProtocolGuid protocol which handles input and displays strings

    INF MsGraphicsPkg/DisplayEngineDxe/DisplayEngineDxe.inf

Application that presents & manages the Boot Menu Setup on Front Page.

    INF OemPkg/BootMenu/BootMenu.inf

Application that presents and manages FrontPage.

    INF OemPkg/FrontPage/FrontPage.inf

Enables MsBootPolicy which implements the desired boot behavior when no UEFI boot options are present (or they
failed) and a alternate boot has been requested (ex. booting from USB).

    INF PcBdsPkg/MsBootPolicy/MsBootPolicy.inf

A module which produces Boot Manager Policy protocol.

    INF MdeModulePkg/Universal/BootManagerPolicyDxe/BootManagerPolicyDxe.inf

A regular expression library to support Remote/IT/Admin Settings such as [DFCI](#what-is-dfci).

    INF MdeModulePkg/Universal/RegularExpressionDxe/RegularExpressionDxe.inf

Processes ingoing and outgoing [DFCI](#what-is-dfci) settings requests

    INF DfciPkg/DfciManager/DfciManager.inf

Driver for On Screen Keyboard.

    INF MsGraphicsPkg/OnScreenKeyboardDxe/OnScreenKeyboardDxe.inf

Spoofs button press to automatically boot to FrontPage.

    INF OemPkg/FrontpageButtonsVolumeUp/FrontpageButtonsVolumeUp.inf

Manages windows and fonts to be drawn by the rendering engine

    INF MsGraphicsPkg/SimpleWindowManagerDxe/SimpleWindowManagerDxe.inf

This final bit places the bmp files and shell.efi file within this firmware volume so they can be
accessed by UEFI drivers.

      FILE APPLICATION=PCD(gPcBdsPkgTokenSpaceGuid.PcdShellFile) {
        SECTION PE32=$(OUTPUT_DIRECTORY)/$(TARGET)_$(TOOL_CHAIN_TAG)/AARCH64/Shell.efi
        SECTION UI= "EdkShell"
      }
      FILE FREEFORM = PCD(gOemPkgTokenSpaceGuid.PcdLogoFile) {
        SECTION RAW = OemPkg/FrontPage/Resources/BootLogo.bmp
        SECTION UI = "Logo"
      }
      FILE FREEFORM = PCD(gOemPkgTokenSpaceGuid.PcdFrontPageLogoFile) {
        SECTION RAW = OemPkg/FrontPage/Resources/FrontpageLogo.bmp
      }
      FILE FREEFORM = PCD(gOemPkgTokenSpaceGuid.PcdLowBatteryFile) {
        SECTION RAW = OemPkg/FrontPage/Resources/LBAT.bmp
      }
      FILE FREEFORM = PCD(gOemPkgTokenSpaceGuid.PcdThermalFile) {
        SECTION RAW = OemPkg/FrontPage/Resources/THOT.bmp
      }
