################################################################################
#
# bm-frontend
#
################################################################################

BM_FRONTEND_VERSION = 81f1f12ea05c048a177ebaf11a0acb8aa42138eb
BM_FRONTEND_SITE = $(call github,mineralos,bm-frontend,$(BM_FRONTEND_VERSION))
BM_FRONTEND_DEPENDENCIES += host-nodejs bm-backend

HOST_NPM = npm_config_prefix=$(HOST_DIR)/ \
	npm_config_cache=$(@D)/.npm \
	npm_config_userconfig=$(BUILD_DIR)/host-nodejs-$(NODEJS_VERSION)/deps/node-inspect/.npmrc \
	PATH=$(BR_PATH) $(HOST_DIR)/bin/npm

define BM_FRONTEND_BUILD_CMDS
	cd $(@D); \
		$(HOST_NPM) install; \
		$(HOST_NPM) run build
endef

define BM_FRONTEND_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/var/www/webif-static; \
	rm -rf $(TARGET_DIR)/var/www/webif-static/*; \
	cp -dpfr $(@D)/build/* $(TARGET_DIR)/var/www/webif-static
endef

$(eval $(generic-package))
