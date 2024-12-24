################################################################################
#
# webrtc-kvm
#
################################################################################

WEBRTC_KVM_VERSION = d618b36a2d5f25fae8d970655b82d0e1c4957cbc
WEBRTC_KVM_SITE_METHOD = git
WEBRTC_KVM_SITE = https://github.com/BigfootACA/webrtc-kvm
WEBRTC_KVM_GIT_SUBMODULES = YES
WEBRTC_KVM_INSTALL_STAGING = YES
WEBRTC_KVM_LICENSE = GPL-3.0+
WEBRTC_KVM_LICENSE_FILES = LICENSE
WEBRTC_KVM_DEPENDENCIES = \
	libdatachannel \
	libmicrohttpd \
	yaml-cpp \
	jsoncpp \
	util-linux-libs \
	openssl

$(eval $(meson-package))
