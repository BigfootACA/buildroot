AntOS for Ant-C V3 IPKVM

User: antc
Password: oh-my-antc

Contents:
  emmc.img:
    Main OS for eMMC / SD Card
    This image require at least 1GiB to flash
  flash-8M.img:
    Bootloader and mtdoops for SPI Nor Flash
    This image require at least 8MiB to flash
  zImage:
    Linux ARM32 compressed bootable kernel
  sun8i-v3-antc-ipkvm.dtb:
    Device Tree Blob for Ant-C V3 IPKVM
  boot.its:
    ITS (Image Tree Source) for rebuild boot slot
    Target partition: eMMC#boot_a eMMC#boot_b
  u-boot-sunxi-with-spl.bin:
    U-Boot bootloader with sunxi SPL
    Target partition: eMMC#bootloader eMMC-Boot SPI-Nor

Software:
  Buildroot:
    Version: 2023.11.1
    Branch: sun8i-v3
    Git: https://github.com/BigfootACA/buildroot
    Config: antc-ipkvm-antos_defconfig
  Linux:
    Version: 6.6.2
    Branch: sun8i-v3-v6.6
    Git: https://github.com/BigfootACA/linux
    Config: antc-ipkvm_defconfig
  U-Boot:
    Version: 2023.10
    Branch: sun8i-v3
    Git: https://github.com/BigfootACA/u-boot
    Config: antc-ipkvm_defconfig
