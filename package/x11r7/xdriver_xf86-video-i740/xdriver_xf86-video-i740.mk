################################################################################
#
# xdriver_xf86-video-i740 -- Intel i740 video driver
#
################################################################################

XDRIVER_XF86_VIDEO_I740_VERSION = 1.1.0
XDRIVER_XF86_VIDEO_I740_SOURCE = xf86-video-i740-$(XDRIVER_XF86_VIDEO_I740_VERSION).tar.bz2
XDRIVER_XF86_VIDEO_I740_SITE = http://xorg.freedesktop.org/releases/individual/driver
XDRIVER_XF86_VIDEO_I740_AUTORECONF = YES
XDRIVER_XF86_VIDEO_I740_DEPENDANCIES = xserver_xorg-server xproto_fontsproto xproto_randrproto xproto_renderproto xproto_videoproto xproto_xextproto xproto_xproto

$(eval $(call AUTOTARGETS,xdriver_xf86-video-i740))
