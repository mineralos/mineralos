################################################################################
#
# im-backend
#
################################################################################

IM_BACKEND_VERSION = ccfe46ce85ba8042e57e3d4272f3ebbd96b8b834
IM_BACKEND_SITE = $(call github,mineralos,im-backend,$(IM_BACKEND_VERSION))
IM_BACKEND_SITE_METHOD = git

define IM_BACKEND_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/var/www/webif; \
	rm -rf $(TARGET_DIR)/var/www/webif/*; \
	mkdir -p $(TARGET_DIR)/var/www/webif/api; \
	cp -dpfr $(@D)/* $(TARGET_DIR)/var/www/webif/api
endef

define INSTALL_IM_TIMESYNC
	$(INSTALL) -D -m 0644 $(IM_BACKEND_PKGDIR)/timesyncd.conf \
		$(TARGET_DIR)/etc/systemd/timesyncd.conf
endef

IM_BACKEND_POST_INSTALL_TARGET_HOOKS += INSTALL_IM_TIMESYNC

$(eval $(generic-package))
