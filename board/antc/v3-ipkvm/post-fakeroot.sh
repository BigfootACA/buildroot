#!/bin/bash
set -ex
if [ -z "${TARGET_DIR}" -o "${TARGET_DIR}" = "/" ]; then
	exit 1
fi
mkdir -p "${TARGET_DIR}/home/antc"
chown 1000:1000 "${TARGET_DIR}/home/antc"
systemctl --root="${TARGET_DIR}" disable \
	systemd-sysupdate.timer \
	systemd-sysupdate-reboot.timer \
	ipmidetectd.service
