#!/bin/bash
set -ex
if ! [ -f "${TARGET_DIR}/THIS_IS_NOT_YOUR_ROOT_FILESYSTEM" ]; then
	exit 1
fi
sed -i 's,/var/run,/run,g' \
	"${TARGET_DIR}/usr/lib/tmpfiles.d/dbus.conf" \
	"${TARGET_DIR}/usr/lib/systemd/system"/*.service
sed -i \
	-e 's,pam_lastlog.so,pam_loginuid.so,g' \
	-e '/pam_console.so/d' \
	"${TARGET_DIR}/etc/pam.d"/*
find "${TARGET_DIR}/usr"/*bin -type f | xargs file | egrep -i 'perl|python' | awk -F: '{print $1}' | xargs rm -fv
rm -f "${TARGET_DIR}/usr/lib/systemd/system"/xfs_scrub_all.{service,timer}
rm -f "${BINARIES_DIR}/emmc.img"
rm -f "${BINARIES_DIR}/misc.vfat"
