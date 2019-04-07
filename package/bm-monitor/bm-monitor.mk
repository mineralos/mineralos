################################################################################
#
# bm-monitor
#
################################################################################

BM_MONITOR_VERSION = local
BM_MONITOR_SITE = $(BM_MONITOR_PKGDIR)/src
BM_MONITOR_SITE_METHOD = local
BM_MONITOR_DEPENDENCIES = jansson

define BM_MONITOR_BUILD_CMDS
	$(TARGET_CC) $(TARGET_CFLAGS) $(TARGET_LDFLAGS) \
		-lpthread -ljansson -o $(@D)/bm-monitor -I$(@D) \
		$(@D)/bm-monitor.c $(@D)/bm-stats.c \
		$(@D)/bm-socket-server.c

endef

define BM_MONITOR_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 755 $(@D)/bm-monitor \
		$(TARGET_DIR)/bin/bm-monitor
endef

define BM_MONITOR_INSTALL_INIT_SYSTEMD
	$(INSTALL) -D -m 0644 $(BM_MONITOR_PKGDIR)/bm-monitor.service \
		$(TARGET_DIR)/usr/lib/systemd/system/bm-monitor.service
	mkdir -p $(TARGET_DIR)/etc/systemd/system/multi-user.target.wants
	ln -fs ../../../../usr/lib/systemd/system/bm-monitor.service \
		$(TARGET_DIR)/etc/systemd/system/multi-user.target.wants/bm-monitor.service
endef

$(eval $(generic-package))
