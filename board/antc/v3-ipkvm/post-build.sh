#!/bin/bash
set -ex
if ! [ -f "${TARGET_DIR}/THIS_IS_NOT_YOUR_ROOT_FILESYSTEM" ]; then
	exit 1
fi
"${HOST_DIR}/bin/arm-sunxi-linux-gnueabihf-gcc" \
	board/antc/v3-ipkvm/slots-upgrade.c \
	-o "${TARGET_DIR}/usr/sbin/slots-upgrade" \
	-lblkid -s -Wall -Wextra -Werror
