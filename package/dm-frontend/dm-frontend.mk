################################################################################
#
# dm-frontend
#
################################################################################

DM_FRONTEND_VERSION = 7c0e096d8d85d8d7a73255e7e0928b660847fa96
DM_FRONTEND_SITE = $(call github,mineralos,dm-frontend,$(DM_FRONTEND_VERSION))
DM_FRONTEND_SITE_METHOD = git
DM_FRONTEND_DEPENDENCIES += host-nodejs dm-backend

HOST_NPM = npm_config_prefix=$(HOST_DIR)/ \
	npm_config_cache=$(@D)/.npm \
	npm_config_userconfig=$(BUILD_DIR)/host-nodejs-$(NODEJS_VERSION)/deps/node-inspect/.npmrc \
	PATH=$(BR_PATH) $(HOST_DIR)/bin/npm

define DM_FRONTEND_BUILD_CMDS
	cd $(@D); \
		$(HOST_NPM) install; \
		$(HOST_NPM) run build
endef

define DM_FRONTEND_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/var/www/webif-static; \
	rm -rf $(TARGET_DIR)/var/www/webif-static/*; \
	cp -dpfr $(@D)/build/* $(TARGET_DIR)/var/www/webif-static
endef

$(eval $(generic-package))
