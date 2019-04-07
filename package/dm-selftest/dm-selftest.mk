################################################################################
#
# dm-monitor
#
################################################################################

DM_SELFTEST_VERSION = local
DM_SELFTEST_SITE = $(DM_SELFTEST_PKGDIR)/src
DM_SELFTEST_SITE_METHOD = local
DM_SELFTEST_DEPENDENCIES = jansson mcompat

define DM_SELFTEST_BUILD_CMDS
	$(TARGET_CC) $(TARGET_CFLAGS) $(TARGET_LDFLAGS) \
		-lpthread -ljansson -o $(@D)/dm-selftest \
		$(@D)/dm-selftest.c -lmcompat_drv

endef

define DM_SELFTEST_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 755 $(@D)/dm-selftest \
		$(TARGET_DIR)/bin/dm-selftest
endef

$(eval $(generic-package))
