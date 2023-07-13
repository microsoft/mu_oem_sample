# Simple Bootable Media Example

Barebones example of how to boot to an EFI Application with no dependencies

## To boot directly to this EFI application perform the following

1. Format a usb drive as FAT32
2. At the root of this usb drive create the following folders: `EFI/Boot/`
3. Build and rename `HelloUefi.efi` as `boot<arch>.efi` and place at `EFI/BOOT/`

    * IA32    - `bootx86.efi`
    * AMD64   - `bootx64.efi`
    * AARCH64 - `bootaa64.efi`

4. On your platform, boot into UEFI Menu and change the boot order to boot USB first
5. Turn off your platform
6. Plug in usb drive
7. Reboot
8. If successful, you should now see `Hello Uefi!` in the top left corner