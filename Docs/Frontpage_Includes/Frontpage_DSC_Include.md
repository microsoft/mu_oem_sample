# DSC Include File in Detail

This readme aims to describe FrontpageDsc.inc in detail, providing details and justification for
the libraries and components included for Frontpage.

## Preliminaries

### What is a DSC File

The .dsc filetype is an EDK2 Platform Description file format. The DSC file must define all libraries,
components and/or modules that will be processed by compiler tool chains, such as the GNU C compiler.
To learn more about it, check the
[specification](https://edk2-docs.gitbook.io/edk-ii-dsc-specification/)
on the tianocore-docs github.

### What is an INF File

The .inf filetype is an EDK2 Platform Description file format. The INF file describes properties of a module,
how it is coded, what it provides, what it depends on, architecture specific items, features, etc. regarding
the module. To learn more about INF files, look at the
[specification](https://edk2-docs.gitbook.io/edk-ii-inf-specification/)
on the tianocore-docs github.

### How do I Include a DSC File

This file will be included in the DSC file for the platform being built. The syntax for including a file
is:

    !include path/to/dsc/include/file

### What are NULL Libs

A NULL lib is one which satisfies a dependency but doesn't implement the required functions.
They're used when a package requires the definition of a library but the way in which the package
is being used does not actually necessitate that library. Instead of reworking the package to remove
dependencies on the unused library, a NULL version of that
libraray can be created which fits the specification but returns the trivial result when called.

### What is DFCI

From the [feature document](https://microsoft.github.io/mu/dyn/mu_plus/DfciPkg/Docs/Dfci_Feature/):
*The Device Firmware Configuration Interface (DFCI) brings new levels of security and usability to
PC configuration management. It is a new feature of UEFI that enables secure programmatic configuration
of hardware settings that are typically configured within a BIOS menu.*

## Libraries included

**NOTE:** Three libraries (MsPlatformDevicesLib, DfciDeviceIdSupportLib, PlatformThemeLib) will need
to be implemented per platform and included either here or in the main DSC file. An example can
be found in the
[NXP iMX8 platform](https://github.com/ms-iot/MU_SILICON_NXP/tree/master/iMX8Pkg).
in the Library/ directory.

### Shell libraries

From the frontpage, the shell can be loaded from frontpage using the boot menu. **ShellLib** provides
the main functionality for shell commands and applications. **ShellCommandLib** provides the internal
support for ShellLib. **ShellCEntryLib** enables a C-style main function with traditional argc and argv
parameters. **HandleParsingLib** supports the parsing the handle and protocol database from the shell.
**BcfgCommandLib** enables the usage of the bcfg command in the shell. Information about bcfg can be found
in the [UEFI Shell Specification v2.2](https://uefi.org/sites/default/files/resources/UEFI_Shell_2_2.pdf)
on page 96. **NetLib** provides basic network functionality.

    ShellLib|ShellPkg/Library/UefiShellLib/UefiShellLib.inf
    ShellCommandLib|ShellPkg/Library/UefiShellCommandLib/UefiShellCommandLib.inf
    ShellCEntryLib|ShellPkg/Library/UefiShellCEntryLib/UefiShellCEntryLib.inf
    HandleParsingLib|ShellPkg/Library/UefiHandleParsingLib/UefiHandleParsingLib.inf
    BcfgCommandLib|ShellPkg/Library/UefiShellBcfgCommandLib/UefiShellBcfgCommandLib.inf
    NetLib|MdeModulePkg/Library/DxeNetLib/DxeNetLib.inf

### Intrinsics for crypto operations

IntrinsicLib is an intrinsic memory routines wrapper implementation for an [OpenSSL](#OpenSSL-Library)
-based cryptographic library.

    IntrinsicLib|CryptoPkg/Library/IntrinsicLib/IntrinsicLib.inf

### [NULL](#what-are-null-libs?) routines to support [DFCI](#what-is-dfci) UI operations

Info on **DfciUiSupportLib** can be found
[here](https://microsoft.github.io/mu/dyn/mu_plus/DfciPkg/Docs/PlatformIntegration/DfciUiSupportLib/)

    DfciUiSupportLib|DfciPkg/Library/DfciUiSupportLibNull/DfciUiSupportLibNull.inf

### Crypto support functions for the [DFCI](#what-is-dfci) recovery feature

Info on **DfciRecoveryLib** can be found
[here](https://microsoft.github.io/mu/dyn/mu_plus/DfciPkg/Docs/PlatformIntegration/DfciUiSupportLib/)

    DfciRecoveryLib|DfciPkg/Library/DfciRecoveryLib/DfciRecoveryLib.inf

### OpenSSL Library

**OpensslLib** is an open-source implementation of the
[SSL and TLS](https://en.wikipedia.org/wiki/Transport_Layer_Security) protocols.
This library is used by BaseCryptLib.

    OpensslLib|CryptoPkg/Library/OpensslLib/OpensslLib.inf

### Stub functions that should contain base logic for querying, setting, and verifying user passwords

**BaseCryptLib** provides basic library functions for cryptographic primitives.

    BaseCryptLib|CryptoPkg/Library/BaseCryptLib/BaseCryptLib.inf

### Supports [DFCI](#what-is-dfci) Groups

**DfciGroupLib** allows settings managers to keep separate configurations depending upon the devices
group membership. More on DfciGroupLib can be found
[here](https://microsoft.github.io/mu/dyn/mu_plus/DfciPkg/Docs/PlatformIntegration/DfciGroups/#dfcigrouplib).

**XmlTreeQueryLib** and **XmlTreeLib** introduce some xml support into UEFI. Info on the
Xml Support Package can be found
[here](https://microsoft.github.io/mu/dyn/mu_plus/XmlSupportPkg/ReadMe/).

    DfciGroupLib|DfciPkg/Library/DfciGroupLibNull/DfciGroups.inf
    XmlTreeQueryLib|XmlSupportPkg/Library/XmlTreeQueryLib/XmlTreeQueryLib.inf
    XmlTreeLib|XmlSupportPkg/Library/XmlTreeLib/XmlTreeLib.inf

### Backwards compatibility with [DFCI](#what-is-dfci) V1 functions

**DfciV1SupportLib** (or rather its [NULL](#what-are-null-libs?) instance) translates V1 DFCI strings
into V2 DFCI strings.

**FltUsedLib** provides a global (fltused) that needs to be defined anywhere floating point operations
are used.

    DfciV1SupportLib|DfciPkg/Library/DfciV1SupportLibNull/DfciV1SupportLibNull.inf
    FltUsedLib|MdePkg/Library/FltUsedLib/FltUsedLib.inf

### Library instances which understand the MsXml Settings Schema

**DfciXmlSettingSchemaSupportLib** supports the xml schema used to store/define settings in
[DFCI](#what-is-dfci).

**DfciXmlPermissionSchemaSupportLib** supports the xml format used to store permissions. More information
on DFCI Permissions can be found
[here](https://microsoft.github.io/mu/dyn/mu_plus/DfciPkg/Docs/Internals/DfciInternals/#permission-packet-formats).

**DfciXmlDeviceIdSchemaSupportLib** and **DfciXmlIdentitySchemaSupportLib** support the xml format used
to store Device Identities.More info on the DFCI Identity Manager can be found
[here](https://microsoft.github.io/mu/dyn/mu_plus/DfciPkg/Docs/Internals/DfciInternals/#identity-manager)

    DfciXmlSettingSchemaSupportLib|DfciPkg/Library/DfciXmlSettingSchemaSupportLib/DfciXmlSettingSchemaSupportLib.inf
    DfciXmlPermissionSchemaSupportLib|DfciPkg/Library/DfciXmlPermissionSchemaSupportLib/DfciXmlPermissionSchemaSupportLib.inf
    DfciXmlDeviceIdSchemaSupportLib|DfciPkg/Library/DfciXmlDeviceIdSchemaSupportLib/DfciXmlDeviceIdSchemaSupportLib.inf
    DfciXmlIdentitySchemaSupportLib|DfciPkg/Library/DfciXmlIdentitySchemaSupportLib/DfciXmlIdentitySchemaSupportLib.inf

### Support for Zero Touch

The term Zero Touch is used in [DFCI](#what-is-dfci) to describe managing device settings remotely.
**ZeroTouchSettingsLib** is a library instance for ZeroTouch to support enabling, display,
and deleting the Zero Touch Certificate.

**SafeIntLib** is a simple integer mathematics library which guards against
[integer overflow](https://en.wikipedia.org/wiki/Integer_overflow).

**BmpSupportLib** supports the use of [bitmap](https://en.wikipedia.org/wiki/BMP_file_format) files.

    ZeroTouchSettingsLib|ZeroTouchPkg/Library/ZeroTouchSettings/ZeroTouchSettings.inf
    SafeIntLib|MdePkg/Library/BaseSafeIntLib/BaseSafeIntLib.inf
    BmpSupportLib|MdeModulePkg/Library/BaseBmpSupportLib/BaseBmpSupportLib.inf

### [NULL](#what-are-null-libs?) library for displaying device state

**DisplayDeviceStateLib** usually supplies a function to display all active device states. This NULL
implementation satisfies dependencies on this function, but no non-null
version of this lib exists in Project Mu.

    DisplayDeviceStateLib|MsGraphicsPkg/Library/DisplayDeviceStateLibNull/DisplayDeviceStateLibNull.inf

### Libraries supporting boot graphics

**BootGraphicsLib** is used by
[BDS](https://en.wikipedia.org/wiki/Unified_Extensible_Firmware_Interface#BDS_-_Boot_Device_Select) to
draw and the main boot graphics to the screen. In the case of Microsoft Surface products, this is
usually the Windows logo.

**BootGraphicsProviderLib** enables the retrieval of the boot graphics used by BootGraphicsLib from
a Firmware Volume.

    BootGraphicsLib|MsGraphicsPkg/Library/BootGraphicsLib/BootGraphicsLib.inf
    BootGraphicsProviderLib|OemPkg/Library/BootGraphicsProviderLib/BootGraphicsProviderLib.inf

### Support libary for MS_UI_THEME

The MS_UI_THEME is the theme used by our FrontPage. **MsUiThemeCopyLib** is used by the graphics package to
copy the stored theme into a buffer.

    MsUiThemeCopyLib|MsGraphicsPkg/Library/MsUiThemeCopyLib/MsUiThemeCopyLib.inf

### Version Support Library

**MuUefiVersionLib** is a simple library which returns the current UEFI version.

    MuUefiVersionLib|OemPkg/Library/MuUefiVersionLib/MuUefiVersionLib.inf

### Thermal Support

This [NULL](#what-are-null-libs?) implementation of the **ThermalServicesLib** satisfies a dependency from
[DeviceBootManagerlib](#device-specific-boot-actions-supporting-platformbootmanagerlib). Because there
is no universal API for fetching thermal state, ThermalServicesLib requires a platform specific
implementation to function.

    ThermalServicesLib|PcBdsPkg/Library/ThermalServicesLibNull/ThermalServicesLibNull.inf

### Platform power services

Note that these are both [NULL](#what-are-null-libs?) implementations.

**PowerServicesLib** supports Platforms that don't have a battery. **MsPlatformPowerCheckLib** allows
platform code to configure CPU power limits.

    PowerServicesLib|PcBdsPkg/Library/PowerServicesLibNull/PowerServicesLibNull.inf
    MsPlatformPowerCheckLib|PcBdsPkg/Library/MsPlatformPowerCheckLibNull/MsPlatformPowerCheckLibNull.inf

### Console Message Library

**ConsoleMsgLib** offers functionality to display platform specific debug messages. Platforms are responsible
for implementing this interface (this is a [NULL](#what-are-null-libs?) implementation)

    ConsoleMsgLib|PcBdsPkg/Library/ConsoleMsgLibNull/ConsoleMsgLibNull.inf

### Console Configuration

**GraphicsConsoleHelperLib** enables the setting of the graphics console resolution to known values (ex. native
resolution, low resolution, etc.).

    GraphicsConsoleHelperLib|PcBdsPkg/Library/GraphicsConsoleHelperLib/GraphicsConsoleHelper.inf

### Device State Support

**DeviceStateLib** is a simple interface for getting and setting the device state (ex. Manufacturing Mode,
Unit Test Mode, etc.). **DisplayDeviceStateLib** uses color bards to display device states during boot.
For example, the following color bar displayed at the top of the screen implies Secure Boot is disabled

![Colorbar](colorbar_mu.PNG)

    DeviceStateLib|MsCorePkg/Library/DeviceStateLib/DeviceStateLib.inf
    DisplayDeviceStateLib|MsGraphicsPkg/Library/ColorBarDisplayDeviceStateLib/ColorBarDisplayDeviceStateLib.inf

### Device specific boot actions supporting PlatformBootManagerLib

**DeviceBootManagerLib** provides extensions for BdsDxe which is responsible for producing the protocol
gEfiBdsArchProtocolGuid which is used to transition between
[DXE](https://en.wikipedia.org/wiki/Unified_Extensible_Firmware_Interface#DXE_-_Driver_Execution_Environment) and
[BDS](https://en.wikipedia.org/wiki/Unified_Extensible_Firmware_Interface#BDS_-_Boot_Device_Select) phases.

    DeviceBootManagerLib|PcBdsPkg/Library/DeviceBootManagerLib/DeviceBootManagerLib.inf

### Rectangle Primitives Library

**UiRectangleLib** enables the drawing of rectangles with different fills and borders.

    UiRectangleLib|MsGraphicsPkg/Library/BaseUiRectangleLib/BaseUiRectangleLib.inf

### Boot Management

**MsAltBootLib** sets and gets the alternate boot variable used to specify when the user wants to boot from
a USB or other device.

**MsBootOptionsLib** extends BdsDxe to allow booting from specific storage devices or to the shell.

    MsAltBootLib|OemPkg/Library/MsAltBootLib/MsAltBootLib.inf
    MsBootOptionsLib|PcBdsPkg/Library/MsBootOptionsLib/MsBootOptionsLib.inf

### Color Support

**MsColorTableLib** is a simple library of colors

    MsColorTableLib|MsGraphicsPkg/Library/MsColorTableLib/MsColorTableLib.inf

### Networking Support

**MsNetworkDependencyLib** is used by the boot manager to start the networking stack.

    MsNetworkDependencyLib|PcBdsPkg/Library/MsNetworkDependencyLib/MsNetworkDependencyLib.inf

### Library to access reboot reason

**MsNVBootReasonLib** provides an interface for retrieving the reboot reason (ex. bootfail) from non-volatile variable storage.

    MsNVBootReasonLib|OemPkg/Library/MsNVBootReasonLib/MsNVBootReasonLib.inf

### Math Library

**MathLib** provides an architecture agnostic math library offering reasonable approximations for various
functions in software.

    MathLib|MsCorePkg/Library/MathLib/MathLib.inf

### System Boot Graphics Support

**BootGraphicsLib** is intended only to be used by
[BDS](https://en.wikipedia.org/wiki/Unified_Extensible_Firmware_Interface#BDS_-_Boot_Device_Select)
to draw the main boot graphics on the screen. (ex. the Microsoft logo).

**BootGraphicsProviderLib** uses PCDs to retrieve BMP files from firmware volumes.

    BootGraphicsLib|MsGraphicsPkg/Library/BootGraphicsLib/BootGraphicsLib.inf
    BootGraphicsProviderLib|OemPkg/Library/BootGraphicsProviderLib/BootGraphicsProviderLib.inf

### Boot Manager Settings Access

**MsBootManagerSettingsLib** provides get and set access for boot manager settings.

    MsBootManagerSettingsLib|PcBdsPkg/Library/MsBootManagerSettingsDxeLib/MsBootManagerSettingsDxeLib.inf

### Simple Window Manager Dialogs

**SwmDialogsLib** enables the display of dialog boxes such as a password input box.

    SwmDialogsLib|MsGraphicsPkg/Library/SwmDialogsLib/SwmDialogs.inf

### Platform Specific Boot Policy

**MsBootPolicyLib** implements the desired boot behavior when no UEFI boot options are present (or they failed)
and a alternate boot has been requested (ex. booting from USB).

    MsBootPolicyLib|OemPkg/Library/MsBootPolicyLib/MsBootPolicyLib.inf

### UI Support

**MsUiThemeLib** Supplies the theme for this platform to the UEFI settings UI. **UIToolKitLib** initializes
toolkit required for UI display.

    MsUiThemeLib|MsGraphicsPkg/Library/MsUiThemeLib/Dxe/MsUiThemeLib.inf
    UIToolKitLib|MsGraphicsPkg/Library/SimpleUIToolKit/SimpleUIToolKit.inf

### Security Support

**ResetUtilityLib** contains various helper functions for resetting the system. **BaseBinSecurityLib** is
a [NULL](#what-are-null-libs?) implementation of a library for initializing security cookies.
**SecurityLockAuditLib** is another NULL implementation to satisfy compilation dependencies. The implemented
version supplies necessary functions for logging hardware and software security locks.

    ResetUtilityLib|MdeModulePkg/Library/ResetUtilityLib/ResetUtilityLib.inf
    BaseBinSecurityLib|MdePkg/Library/BaseBinSecurityLibNull/BaseBinSecurityLibNull.inf
    SecurityLockAuditLib|MdeModulePkg/Library/SecurityLockAuditLibNull/SecurityLockAuditLibNull.inf

## PCDs included

Set to TRUE so Console In (ConIn. ex. keyboard) is only connected once a console input is requested
(ex. when a device is plugged).

    gEfiMdeModulePkgTokenSpaceGuid.PcdConInConnectOnDemand|TRUE

Sets maximum printable number of characters to 16000

    gEfiMdePkgTokenSpaceGuid.PcdUefiLibMaxPrintBufferSize|16000

Used to control the profiles available in the shell. 0x1f sets all profiles to available.

    gEfiShellPkgTokenSpaceGuid.PcdShellProfileMask|0x1f     # All profiles

If TRUE, MsUiTheme is built in
[DXE](https://en.wikipedia.org/wiki/Unified_Extensible_Firmware_Interface#DXE_-_Driver_Execution_Environment).
This should be TRUE for platforms that don't have a PPI
([PEI](https://en.wikipedia.org/wiki/Unified_Extensible_Firmware_Interface#PEI_-_Pre-EFI_Initialization)
module to PEI module Interface). Otherwise, the theme will be generated in PEI and it will be located
on a HOB.

    gMsGraphicsPkgTokenSpaceGuid.PcdUiThemeInDxe|TRUE

If TRUE, the
[Boot Manager](https://docs.microsoft.com/en-us/windows-hardware/drivers/bringup/boot-and-uefi#understanding-the-windows-boot-manager)
will be in the boot order list.

    gEfiMdeModulePkgTokenSpaceGuid.PcdBootManagerInBootOrder|TRUE

If TRUE, the
[BDS](https://en.wikipedia.org/wiki/Unified_Extensible_Firmware_Interface#BDS_-_Boot_Device_Select)
supports Platform Recovery

    gEfiMdeModulePkgTokenSpaceGuid.PcdPlatformRecoverySupport|FALSE

Set to the guid associated with the Mu Frontpage.

    gEfiMdeModulePkgTokenSpaceGuid.PcdBootManagerMenuFile|{ 0x8A, 0x70, 0x42, 0x40, 0x2D, 0x0F, 0x23, 0x48, 0xAC, 0x60, 0x0D, 0x77, 0xB3, 0x11, 0x18, 0x89 }

## Components

Spoofs button press to automatically boot to FrontPage.

    OemPkg/FrontpageButtonsVolumeUp/FrontpageButtonsVolumeUp.inf

Application that presents and manages FrontPage.

    OemPkg/FrontPage/FrontPage.inf

Application that presents & manages the Boot Menu Setup on Front Page.

    OemPkg/BootMenu/BootMenu.inf

Enables MsBootPolicy. See the [Platform Specific Boot Policy Library](#platform-specific-boot-policy)

    PcBdsPkg/MsBootPolicy/MsBootPolicy.inf

A module which produces Boot Manager Policy protocol.

    MdeModulePkg/Universal/BootManagerPolicyDxe/BootManagerPolicyDxe.inf

A regular expression library to support Remote/IT/Admin Settings such as [DFCI](#what-is-dfci).

    MdeModulePkg/Universal/RegularExpressionDxe/RegularExpressionDxe.inf

Main component of [DFCI](#what-is-dfci) which manages and enforces all DFCI settings

    DfciPkg/SettingsManager/SettingsManagerDxe.inf {
        # Platform should add all it settings libs here
        <LibraryClasses>
            NULL|ZeroTouchPkg/Library/ZeroTouchSettings/ZeroTouchSettings.inf
            NULL|DfciPkg/Library/DfciPasswordProvider/DfciPasswordProvider.inf
            NULL|DfciPkg/Library/DfciSettingsLib/DfciSettingsLib.inf
            DfciSettingPermissionLib|DfciPkg/Library/DfciSettingPermissionLib/DfciSettingPermissionLib.inf
            NULL|PcBdsPkg/Library/MsBootManagerSettingsDxeLib/MsBootManagerSettingsDxeLib.inf
        <PcdsFeatureFlag>
            gDfciPkgTokenSpaceGuid.PcdSettingsManagerInstallProvider|TRUE
    }

AuthManager provides authentication for [DFCI](#what-is-dfci).

    DfciPkg/IdentityAndAuthManager/IdentityAndAuthManagerDxe.inf

Processes ingoing and outgoing [DFCI](#what-is-dfci) settings requests

    DfciPkg/DfciManager/DfciManager.inf

Manages windows and fonts to be drawn by the rendering engine

    MsGraphicsPkg/SimpleWindowManagerDxe/SimpleWindowManagerDxe.inf

Produces EfiGraphicsOutputProtocol to draw graphics to the screen

    MsGraphicsPkg/RenderingEngineDxe/RenderingEngineDxe.inf

Finds and takes ownership of gEfiGraphicsOutputProtocolGuid so RenderingEngine can use it by uninstalling
it on its current handle and reinstalling it on a new one.

    MsGraphicsPkg/GopOverrideDxe/GopOverrideDxe.inf

Driver for On Screen Keyboard.

    MsGraphicsPkg/OnScreenKeyboardDxe/OnScreenKeyboardDxe.inf

Installs protocol to share the UI theme.

    MsGraphicsPkg/MsUiTheme/Dxe/MsUiThemeProtocol.inf

Produces gEdkiiFormDisplayEngineProtocolGuid protocol. Handles input, displays strings

    MsGraphicsPkg/DisplayEngineDxe/DisplayEngineDxe.inf

Enables the Shell

    ShellPkg/Application/Shell/Shell.inf {
        <LibraryClasses>
            NULL|ShellPkg/Library/UefiShellLevel2CommandsLib/UefiShellLevel2CommandsLib.inf
            NULL|ShellPkg/Library/UefiShellLevel1CommandsLib/UefiShellLevel1CommandsLib.inf
            NULL|ShellPkg/Library/UefiShellLevel3CommandsLib/UefiShellLevel3CommandsLib.inf
            NULL|ShellPkg/Library/UefiShellDriver1CommandsLib/UefiShellDriver1CommandsLib.inf
            NULL|ShellPkg/Library/UefiShellInstall1CommandsLib/UefiShellInstall1CommandsLib.inf
            NULL|ShellPkg/Library/UefiShellDebug1CommandsLib/UefiShellDebug1CommandsLib.inf
            NULL|ShellPkg/Library/UefiShellNetwork1CommandsLib/UefiShellNetwork1CommandsLib.inf
            NULL|ShellPkg/Library/UefiShellNetwork2CommandsLib/UefiShellNetwork2CommandsLib.inf
        <PcdsFixedAtBuild>
            gEfiShellPkgTokenSpaceGuid.PcdShellLibAutoInitialize|FALSE
    }
