################################################################################
#
# dm-backend
#
################################################################################

DM_BACKEND_VERSION = 21970cc5ede8dd413792a533ad463f024183f5fa
DM_BACKEND_SITE = $(call github,mineralos,dm-backend,$(DM_BACKEND_VERSION))
DM_BACKEND_SITE_METHOD = git

define DM_BACKEND_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/var/www/webif; \
	rm -rf $(TARGET_DIR)/var/www/webif/*; \
	mkdir -p $(TARGET_DIR)/var/www/webif/api; \
	cp -dpfr $(@D)/* $(TARGET_DIR)/var/www/webif/api
endef

define INSTALL_DM_TIMESYNC
	$(INSTALL) -D -m 0644 $(DM_BACKEND_PKGDIR)/timesyncd.conf \
		$(TARGET_DIR)/etc/systemd/timesyncd.conf
endef

DM_BACKEND_POST_INSTALL_TARGET_HOOKS += INSTALL_DM_TIMESYNC

$(eval $(generic-package))
