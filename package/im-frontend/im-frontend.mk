################################################################################
#
# im-frontend
#
################################################################################

IM_FRONTEND_VERSION = 2ecd5bb8681592c349d8a84f6c87bab59bb0e4b4
IM_FRONTEND_SITE = $(call github,mineralos,im-frontend,$(IM_FRONTEND_VERSION))
IM_FRONTEND_SITE_METHOD = git
IM_FRONTEND_DEPENDENCIES += host-nodejs im-backend

HOST_NPM = npm_config_prefix=$(HOST_DIR)/ \
	npm_config_cache=$(@D)/.npm \
	npm_config_userconfig=$(BUILD_DIR)/host-nodejs-$(NODEJS_VERSION)/deps/node-inspect/.npmrc \
	PATH=$(BR_PATH) $(HOST_DIR)/bin/npm

define IM_FRONTEND_BUILD_CMDS
	cd $(@D); \
		$(HOST_NPM) install; \
		$(HOST_NPM) run build
endef

define IM_FRONTEND_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/var/www/webif-static; \
	rm -rf $(TARGET_DIR)/var/www/webif-static/*; \
	cp -dpfr $(@D)/build/* $(TARGET_DIR)/var/www/webif-static
endef

$(eval $(generic-package))
