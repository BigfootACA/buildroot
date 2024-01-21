#!/bin/bash
set -ex
cp board/antc/v3-ipkvm/boot.its "${BINARIES_DIR}"
(cd "${BINARIES_DIR}" && mkimage -f boot.its boot.itb)
support/scripts/genimage.sh -c board/antc/v3-ipkvm/genimage.cfg
if [ -f "${BINARIES_DIR}/emmc.img" ]; then
	fallocate -l 360M "${BINARIES_DIR}/emmc.img"
	echo "NOTICE: you need at least 1GiB to flash this image"
fi
