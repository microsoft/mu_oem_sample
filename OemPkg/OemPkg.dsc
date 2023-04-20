## @file
# Oem Sample Package Localized Libraries and Content
#
#  Copyright (C) Microsoft Corporation. All rights reserved.
#  SPDX-License-Identifier: BSD-2-Clause-Patent
#
##

[Defines]
  PLATFORM_NAME                  = OemPkg
  PLATFORM_GUID                  = 17820FE5-2B34-4B97-80E8-FB435FC3E8DD
  PLATFORM_VERSION               = 1.0
  DSC_SPECIFICATION              = 0x00010005
  OUTPUT_DIRECTORY               = Build/OemPkg
  SUPPORTED_ARCHITECTURES        = IA32|X64
  BUILD_TARGETS                  = DEBUG|RELEASE
  SKUID_IDENTIFIER               = DEFAULT

[PcdsFeatureFlag]


[PcdsFixedAtBuild]

!include MdePkg/MdeLibs.dsc.inc

[LibraryClasses.common]
  BaseLib|MdePkg/Library/BaseLib/BaseLib.inf
  BaseMemoryLib|MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
  DebugLib|MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf
  UefiLib|MdePkg/Library/UefiLib/UefiLib.inf
  UefiDriverEntryPoint|MdePkg/Library/UefiDriverEntryPoint/UefiDriverEntryPoint.inf
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  ReportStatusCodeLib|MdePkg/Library/BaseReportStatusCodeLibNull/BaseReportStatusCodeLibNull.inf
  UefiApplicationEntryPoint|MdePkg/Library/UefiApplicationEntryPoint/UefiApplicationEntryPoint.inf

  PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
  HiiLib|MdeModulePkg/Library/UefiHiiLib/UefiHiiLib.inf
  UefiBootServicesTableLib|MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
  UefiRuntimeServicesTableLib|MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
  UefiHiiServicesLib|MdeModulePkg/Library/UefiHiiServicesLib/UefiHiiServicesLib.inf
  UefiBootManagerLib|MdeModulePkg/Library/UefiBootManagerLib/UefiBootManagerLib.inf
  MemoryTypeInformationChangeLib|MdeModulePkg/Library/MemoryTypeInformationChangeLibNull/MemoryTypeInformationChangeLibNull.inf  # MU_CHANGE
  DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
  DxeServicesLib|MdePkg/Library/DxeServicesLib/DxeServicesLib.inf
  SafeIntLib|MdePkg/Library/BaseSafeIntLib/BaseSafeIntLib.inf
  BmpSupportLib|MdeModulePkg/Library/BaseBmpSupportLib/BaseBmpSupportLib.inf
  PerformanceLib|MdePkg/Library/BasePerformanceLibNull/BasePerformanceLibNull.inf
  PeCoffGetEntryPointLib|MdePkg/Library/BasePeCoffGetEntryPointLib/BasePeCoffGetEntryPointLib.inf
  DxeServicesTableLib|MdePkg/Library/DxeServicesTableLib/DxeServicesTableLib.inf
  SortLib|MdeModulePkg/Library/UefiSortLib/UefiSortLib.inf
  ResetUtilityLib|MdeModulePkg/Library/ResetUtilityLib/ResetUtilityLib.inf
  ResetSystemLib|MdeModulePkg/Library/DxeResetSystemLib/DxeResetSystemLib.inf
  TimerLib|MdePkg/Library/BaseTimerLibNullTemplate/BaseTimerLibNullTemplate.inf
  VariablePolicyHelperLib|MdeModulePkg/Library/VariablePolicyHelperLib/VariablePolicyHelperLib.inf
  DeviceStateLib|MdeModulePkg/Library/DeviceStateLib/DeviceStateLib.inf

  UIToolKitLib|MsGraphicsPkg/Library/SimpleUIToolKit/SimpleUIToolKit.inf
  MsUiThemeLib|MsGraphicsPkg/Library/MsUiThemeLib/Dxe/MsUiThemeLib.inf
  BootGraphicsLib|MsGraphicsPkg/Library/BootGraphicsLibNull/BootGraphicsLib.inf
  BootGraphicsProviderLib|MsGraphicsPkg/Library/BootGraphicsProviderLibNull/BootGraphicsProviderLib.inf
  MsColorTableLib|MsGraphicsPkg/Library/MsColorTableLib/MsColorTableLib.inf
  SwmDialogsLib|MsGraphicsPkg/Library/SwmDialogsLib/SwmDialogs.inf

  GraphicsConsoleHelperLib|PcBdsPkg/Library/GraphicsConsoleHelperLib/GraphicsConsoleHelper.inf
  MsBootOptionsLib|PcBdsPkg/Library/MsBootOptionsLib/MsBootOptionsLib.inf
  MsPlatformDevicesLib|PcBdsPkg/Library/MsPlatformDevicesLibNull/MsPlatformDevicesLibNull.inf

  MsAltBootLib|OemPkg/Library/MsAltBootLib/MsAltBootLib.inf
  MsBootPolicyLib|OemPkg/Library/MsBootPolicyLib/MsBootPolicyLib.inf
  MsNVBootReasonLib|OemPkg/Library/MsNVBootReasonLib/MsNVBootReasonLib.inf
  MuUefiVersionLib|OemPkg/Library/MuUefiVersionLib/MuUefiVersionLib.inf
  PasswordStoreLib|OemPkg/Library/PasswordStoreLib/PasswordStoreLib.inf
  PasswordPolicyLib|OemPkg/Library/PasswordPolicyLibNull/PasswordPolicyLibNull.inf
  SecureBootVariableLib|SecurityPkg/Library/SecureBootVariableLib/SecureBootVariableLib.inf
  MuSecureBootKeySelectorLib|MsCorePkg/Library/MuSecureBootKeySelectorLib/MuSecureBootKeySelectorLib.inf
  SecureBootKeyStoreLib|OemPkg/Library/SecureBootKeyStoreLibOem/SecureBootKeyStoreLibOem.inf
  DeviceBootManagerLib|MsCorePkg/Library/DeviceBootManagerLibNull/DeviceBootManagerLibNull.inf
  MathLib|MsCorePkg/Library/MathLib/MathLib.inf
  FltUsedLib|MdePkg/Library/FltUsedLib/FltUsedLib.inf

  RngLib|MdePkg/Library/BaseRngLib/BaseRngLib.inf
  PlatformPKProtectionLib|SecurityPkg/Library/PlatformPKProtectionLibVarPolicy/PlatformPKProtectionLibVarPolicy.inf

  ConfigVariableListLib|SetupDataPkg/Library/ConfigVariableListLib/ConfigVariableListLib.inf
  ActiveProfileIndexSelectorLib|OemPkg/Library/ActiveProfileIndexSelectorPcdLib/ActiveProfileIndexSelectorPcdLib.inf

[LibraryClasses.IA32]
  MsUiThemeLib|MsGraphicsPkg/Library/MsUiThemeLib/Pei/MsUiThemeLib.inf

[LibraryClasses.X64]


!if $(TOOL_CHAIN_TAG) == VS2019 or $(TOOL_CHAIN_TAG) == VS2022
  # Provide StackCookie support lib so that we can link to /GS exports for VS builds
  NULL|MdePkg/Library/BaseBinSecurityLibRng/BaseBinSecurityLibRng.inf
  BaseBinSecurityLib|MdePkg/Library/BaseBinSecurityLibRng/BaseBinSecurityLibRng.inf
!else
  BaseBinSecurityLib|MdePkg/Library/BaseBinSecurityLibNull/BaseBinSecurityLibNull.inf
!endif

[LibraryClasses.common.DXE_DRIVER, LibraryClasses.common.UEFI_APPLICATION]
  HobLib|MdePkg/Library/DxeHobLib/DxeHobLib.inf

[LibraryClasses.common.PEIM]
  PeimEntryPoint|MdePkg/Library/PeimEntryPoint/PeimEntryPoint.inf
  PeiServicesLib|MdePkg/Library/PeiServicesLib/PeiServicesLib.inf
  PeiServicesTablePointerLib|MdePkg/Library/PeiServicesTablePointerLib/PeiServicesTablePointerLib.inf
  HobLib|MdePkg/Library/PeiHobLib/PeiHobLib.inf
  MemoryAllocationLib|MdePkg/Library/PeiMemoryAllocationLib/PeiMemoryAllocationLib.inf
  ConfigKnobShimLib|SetupDataPkg/Library/ConfigKnobShimLib/ConfigKnobShimPeiLib/ConfigKnobShimPeiLib.inf
###############################################################
#
# Components Section - list of the modules and components that will be processed by compilation
#                      tools and the EDK II tools to generate PE32/PE32+/Coff image files.
#
# Note: The EDK II DSC file is not used to specify how compiled binary images get placed
#       into firmware volume images. This section is just a list of modules to compile from
#       source into UEFI-compliant binaries.
#       It is the FDF file that contains information on combining binary files into firmware
#       volume images, whose concept is beyond UEFI and is described in PI specification.
#       Binary modules do not need to be listed in this section, as they should be
#       specified in the FDF file. For example: Shell binary (Shell_Full.efi), FAT binary (Fat.efi),
#       Logo (Logo.bmp), and etc.
#       There may also be modules listed in this section that are not required in the FDF file,
#       When a module listed here is excluded from FDF file, then UEFI-compliant binary will be
#       generated for it, but the binary will not be put into any firmware volume.
#
###################################################################################################
[Components]
  OemPkg/Library/MsAltBootLib/MsAltBootLib.inf
  OemPkg/Library/MsBootPolicyLib/MsBootPolicyLib.inf
  OemPkg/Library/MsNVBootReasonLib/MsNVBootReasonLib.inf
  OemPkg/Library/MuUefiVersionLib/MuUefiVersionLib.inf
  OemPkg/Library/BootGraphicsProviderLib/BootGraphicsProviderLib.inf
  OemPkg/Library/PasswordStoreLib/PasswordStoreLib.inf
  OemPkg/Library/PasswordPolicyLib/PasswordPolicyLib.inf
  OemPkg/Library/PasswordPolicyLibNull/PasswordPolicyLibNull.inf
  OemPkg/Library/MsSecureBootModeSettingLib/MsSecureBootModeSettingLib.inf
  OemPkg/Library/DfciUiSupportLib/DfciUiSupportLib.inf
  OemPkg/Library/DfciGroupLib/DfciGroups.inf
  OemPkg/Library/DfciDeviceIdSupportLib/DfciDeviceIdSupportLib.inf
  OemPkg/Library/SecureBootKeyStoreLibOem/SecureBootKeyStoreLibOem.inf
  OemPkg/Library/OemMfciLib/OemMfciLibPei.inf
  OemPkg/Library/OemMfciLib/OemMfciLibDxe.inf
  OemPkg/FrontpageButtonsVolumeUp/FrontpageButtonsVolumeUp.inf
  OemPkg/Library/PlatformConfigDataNullLib/PlatformConfigDataNullLib.inf
  OemPkg/OemConfigPolicyCreatorPei/OemConfigPolicyCreatorPei.inf {
    <LibraryClasses>
      # platform data lib
      NULL|OemPkg/Library/PlatformConfigDataNullLib/PlatformConfigDataNullLib.inf
  }
  OemPkg/Library/ActiveProfileIndexSelectorPcdLib/ActiveProfileIndexSelectorPcdLib.inf

[Components.IA32]
  OemPkg/DeviceStatePei/DeviceStatePei.inf

[Components.X64]
  OemPkg/FrontPage/FrontPage.inf
  OemPkg/BootMenu/BootMenu.inf
  OemPkg/Override/MdeModulePkg/Universal/BootManagerPolicyDxe/BootManagerPolicyDxe.inf
  OemPkg/Library/DeviceBootManagerLib/DeviceBootManagerLib.inf
  OemPkg/Library/MsBootManagerSettingsDxeLib/MsBootManagerSettingsDxeLib.inf
  OemPkg/Library/UpdateFacsHardwareSignatureLib/UpdateFacsHardwareSignatureLib.inf

[BuildOptions]
#force deprecated interfaces off
  *_*_*_CC_FLAGS = -D DISABLE_NEW_DEPRECATED_INTERFACES
