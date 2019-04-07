################################################################################
#
# bm-backend
#
################################################################################

BM_BACKEND_VERSION = 7b4fa34b0deccee2d2e4d97ec2ad403db48df3a4
BM_BACKEND_SITE = $(call github,mineralos,bm-backend,$(BM_BACKEND_VERSION))

define BM_BACKEND_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/var/www/webif; \
	rm -rf $(TARGET_DIR)/var/www/webif/*; \
	mkdir -p $(TARGET_DIR)/var/www/webif/api; \
	cp -dpfr $(@D)/* $(TARGET_DIR)/var/www/webif/api
endef

define INSTALL_BM_TIMESYNC
	$(INSTALL) -D -m 0644 $(BM_BACKEND_PKGDIR)/timesyncd.conf \
		$(TARGET_DIR)/etc/systemd/timesyncd.conf
endef

BM_BACKEND_POST_INSTALL_TARGET_HOOKS += INSTALL_BM_TIMESYNC

$(eval $(generic-package))
