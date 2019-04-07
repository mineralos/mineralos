################################################################################
#
# dm-monitor
#
################################################################################

DM_MONITOR_VERSION = local
DM_MONITOR_SITE = $(DM_MONITOR_PKGDIR)/src
DM_MONITOR_SITE_METHOD = local
DM_MONITOR_DEPENDENCIES = jansson mcompat

define DM_MONITOR_BUILD_CMDS
	$(TARGET_CC) $(TARGET_CFLAGS) $(TARGET_LDFLAGS) \
		-lpthread -ljansson -o $(@D)/dm-monitor -I$(@D) \
		$(@D)/dm-monitor.c $(@D)/dm-stats.c \
		$(@D)/dm-socket-server.c -lmcompat_drv

endef

define DM_MONITOR_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 755 $(@D)/dm-monitor \
		$(TARGET_DIR)/bin/dm-monitor
endef

define DM_MONITOR_INSTALL_INIT_SYSTEMD
	$(INSTALL) -D -m 0644 $(DM_MONITOR_PKGDIR)/dm-monitor.service \
		$(TARGET_DIR)/usr/lib/systemd/system/dm-monitor.service
	mkdir -p $(TARGET_DIR)/etc/systemd/system/multi-user.target.wants
	ln -fs ../../../../usr/lib/systemd/system/dm-monitor.service \
		$(TARGET_DIR)/etc/systemd/system/multi-user.target.wants/dm-monitor.service
endef

$(eval $(generic-package))
