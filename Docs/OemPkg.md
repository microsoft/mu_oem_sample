# OemPkg

OemPkg contains the logic required to use Project Mu. Though this package should be bootable with some
extra care from a developer, the code should be fully tailored to meet the needs of the product. Below is
a short summary of each piece to the Project Mu puzzle to help jump-start the customization process.

## FrontPage

![FrontPage emulated through QEMU](q35_mu.gif)

FrontPage is Project Mu's UEFI landing page. From here, the user can access items like system
information, boot menu, and
[DFCI](https://microsoft.github.io/mu/dyn/mu_plus/DfciPkg/Docs/Dfci_Feature/) management
settings. There's a significant amount of logic contained within.

**FrontPage.c** is the actual implementation of the FrontPage. It includes all the logic required to
call, update, and populate the FrontPage with system information. Adding or removing elements from the
FrontPage can be done by editing mFormMap.

**FrontPageConfigAccess.c** implements trivial versions of RouteConfig and ExtractConfig to satisfy
dependencies.

**FrontPageStrings.uni** contains all static strings displayed on the UEFI FrontPage.

**FrontPageUi.c** handles updates to the FrontPage UI including updates to the current page and info/popup
boxes.

**FrontPageVfr.Vfr** A VFR (Visual Forms Representation) file defines the layout of a UI and, in this case,
FrontPage. **FrontPageVfr.h** contains guid definitions used in VFR files.

**String.c** contains the logic for fetching strings using HII (Human Interface Infrastructure) protocols.
HII is detailed in the [UEFI Spec v2.8](https://uefi.org/sites/default/files/resources/UEFI_Spec_2_8_final.pdf)
on page 1731.

## FrontpageButtonsVolumeUp

By default, booting to UEFI requires holding down the volume up (or equivalent) button on the device.
It's unlikely that the volume up button will be appropriate or applicable to most devices, so it's
recommended to alter the trigger for booting to UEFI. This module simulates holding down the volume
up button on every boot so the UEFI FrontPage is loaded every boot.

## BootMenu

The BootMenu on the UEFI FrontPage is under the *Boot configuration* tab. It defines the boot order
for the device and advanced options which can potentially be managed through
[DFCI](https://microsoft.github.io/mu/dyn/mu_plus/DfciPkg/Docs/Dfci_Feature/).

**BootMenu.c** contains all logic required by the BootMenu including changing settings (assuming they
are not locked through DFCI) and rebuilding the boot order.

**BootMenuStrings.uni** contains all static strings displayed on the BootMenu.

**BootMenuVfr.Vfr** defines the layout of the BootMenu UI. **BootMenu.h** contains guid definitions
used in the VFR file.

## DeviceStatePei

This module sets the Device State bits in the
[PEI](https://en.wikipedia.org/wiki/Unified_Extensible_Firmware_Interface#PEI_-_Pre-EFI_Initialization)
phase so they are available prior to display in the
[DXE](https://en.wikipedia.org/wiki/Unified_Extensible_Firmware_Interface#DXE_-_Driver_Execution_Environment)
phase. Possible Device States include Manufacturing Mode Enabled/Disabled, Unit Test Mode, Secure
Boot Enabled/Disabled, etc.

## Include(s)

As is standard across [EDK2](https://github.com/tianocore/edk2), the Include/ directory contains header
files for functionality under the Library/ directory (not to be confused with Include/Library/) and is
available for inclusion in other modules.

**MsNVBootReason.h** provides an interface for retrieving the reboot reason (ex. bootfail) from
non-volatile variable storage.

**PasswordStoreVariable.h** defines the GUID and variable names for a variable-backed PasswordStore.

**PasswordPolicyLib.h** contains the interface for storing and hashing an administrator password.

**ButtonServices.h** is the header for [FrontpageButtonsVolumeUp.c](#FrontpageButtonsVolumeUp)

**MsFrontPageAuthTokenProtocol.h** is required to access the authentication token generated when
FrontPage is launched. This token is used in all FrontPage applications to retrieve data from the
settings provider.

**FrontPageSettings.h** contains some variables correlating with settings on FrontPage.

## Library

As is standard across [EDK2](https://github.com/tianocore/edk2), the Library/ directory contains actual
implementations of functionality provided/required by this module.

**BootGraphicsProviderLib** enables the retrieval of the boot graphics used by BootGraphicsLib from
a Firmware Volume.

**DfciDeviceIdSupportLib** provides access to platform data that becomes the DFCI Device ID which include
the manufacturer name, product name, and serial number. Device IDs are used to target devices with
DFCI settings management.

**DfciGroupLib** allows settings managers to keep separate configurations depending upon the devices
group membership. More on DfciGroupLib can be found
[here](https://microsoft.github.io/mu/dyn/mu_plus/DfciPkg/Docs/PlatformIntegration/DfciGroups/#dfcigrouplib).

**DfciUiSupportLib** allows DFCI to communicate with the user during DFCI initialization, enrollment,
or to indicate a non secure environment is available.

**MsAltBootLib** sets and gets the alternate boot variable used to specify when the user wants to
boot from a USB or other device.

**MsBootPolicyLib** implements the desired boot behavior when no UEFI boot options are present (or
they failed) and a alternate boot has been requested (ex. booting from USB).

**MsSecureBootModeSettingLib** sets and gets the Secure Boot mode value during the
[DXE](https://en.wikipedia.org/wiki/Unified_Extensible_Firmware_Interface#DXE_-_Driver_Execution_Environment)
phase of execution.

**MsUefiVersionLib** simply provides platform version information.

**PasswordPolicyLib** contains the logic for storing and hashing an administrator password.

**PasswordPolicyLibNull** is the NULL version of PasswordPolicyLib used when the actual functionality
is unnecessary but some other component requires the library definition to successfully build.

**PasswordStoreLib** manages storage location for the platform administrator password.

**PlatformKeyLibNull** is the NULL implementation of PlatformKeyLib to satisfy dependencies.

## Override

The Override/ directory contains overrides for EDK2 components. These overrides are sometimes required
for things like bug fixes, functionality addition and removal. In this case, the only override is for
**BootManagerPolicyDxe** to preserve some functionality of the original in case it is changed in the
EDK2 upstream.

## Others

.dec and .dsc files are required by the build process for any package in EDK2, hence the inclusion of
**OemPkg.dsc** and **OemPkg.dec**. The DSC file must define all libraries, components and/or modules
that will be processed by compiler tool chains, such as the GNU C compiler. The DEC file
consists of sections delineated by section tags enclosed within square brackets which are used to
generate AutoGen.c and AutoGen.h files for the EDK2 build infrastructure. For a more in-depth look
at DSC and DEC files, check out the [DSC specification](https://edk2-docs.gitbook.io/edk-ii-dsc-specification/)
and the [DEC specification](https://edk2-docs.gitbook.io/edk-ii-dec-specification/).
