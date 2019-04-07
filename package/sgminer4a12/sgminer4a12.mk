################################################################################
#
# sgminer
#
################################################################################

SGMINER4A12_VERSION = 90316aba07a6b540c200ced3cf364e555fbba979
SGMINER4A12_SITE = $(call github,mineralos,sgminer4a12,$(SGMINER4A12_VERSION))
SGMINER4A12_DEPENDENCIES = host-pkgconf jansson mcompat libcurl
SGMINER4A12_AUTORECONF = YES
SGMINER4A12_CONF_ENV += LDFLAGS="$(TARGET_LDFLAGS) -lmcompat_drv "
SGMINER4A12_CONF_ENV += LIBS='-lmcompat_drv'
SGMINER4A12_CONF_OPTS = --enable-coinflex --with-system-jansson --without-curses --enable-curl

ifeq ($(BR2_INIT_SYSTEMD),y)
SGMINER4A12_CONF_OPTS += --enable-libsystemd
endif

define SGMINER4A12_INSTALL_INIT_SYSTEMD
	$(INSTALL) -D -m 0644 $(SGMINER4A12_PKGDIR)/sgminer4a12.service \
		$(TARGET_DIR)/usr/lib/systemd/system/cgminer.service
	mkdir -p $(TARGET_DIR)/etc/systemd/system/multi-user.target.wants
	ln -fs ../../../../usr/lib/systemd/system/cgminer.service \
		$(TARGET_DIR)/etc/systemd/system/multi-user.target.wants/cgminer.service
endef

define BUILD_SGMINER4A12_API
	$(TARGET_CC) $(TARGET_CFLAGS) $(TARGET_LDFLAGS) -o $(@D)/cgminer-api \
		$(@D)/api-example.c
endef

SGMINER4A12_POST_BUILD_HOOKS += BUILD_SGMINER4A12_API

define RENAME_SGMINER4A12_BIN
	mv $(TARGET_DIR)/usr/bin/sgminer $(TARGET_DIR)/usr/bin/cgminer
endef

define INSTALL_SGMINER4A12_CONFIG
	$(INSTALL) -D -m 0644 $(SGMINER4A12_PKGDIR)/cgminer.conf.default \
		$(TARGET_DIR)/etc/cgminer.conf.default
endef

define INSTALL_B52_HWREVISION
	$(INSTALL) -D -m 0644 $(SGMINER4A12_PKGDIR)/hwrevision_b52 \
		$(TARGET_DIR)/etc/hwrevision
endef

define INSTALL_S11_HWREVISION
	$(INSTALL) -D -m 0644 $(SGMINER4A12_PKGDIR)/hwrevision_s11 \
		$(TARGET_DIR)/etc/hwrevision
endef

define INSTALL_A12_CONF
	$(INSTALL) -D -m 0644 $(SGMINER4A12_PKGDIR)/b52.conf \
		$(TARGET_DIR)/etc/miner.conf
endef

define SGMINER4A12_POST_RSYNC
	cd $(SGMINER4A12_OVERRIDE_SRCDIR) && git show --pretty=format:'%H' | head -n 1 > $(@D)/cgminer_git.hash && cd -
endef

SGMINER4A12_POST_RSYNC_HOOKS += SGMINER4A12_POST_RSYNC

define INSTALL_SGMINER4A12_VERSION
        $(INSTALL) -D -m 0644 $(@D)/cgminer_git.hash \
                $(TARGET_DIR)/etc/cgminer_git.hash
endef

define INSTALL_SGMINER4A12_API
	$(INSTALL) -D -m 755 $(@D)/cgminer-api \
		$(TARGET_DIR)/usr/bin/cgminer-api
endef

SGMINER4A12_POST_INSTALL_TARGET_HOOKS += INSTALL_SGMINER4A12_API RENAME_SGMINER4A12_BIN INSTALL_SGMINER4A12_CONFIG INSTALL_A12_CONF INSTALL_SGMINER4A12_VERSION

ifeq ($(BR2_PACKAGE_SGMINER_S11),y)
SGMINER4A12_POST_INSTALL_TARGET_HOOKS += INSTALL_S11_HWREVISION
else
SGMINER4A12_POST_INSTALL_TARGET_HOOKS += INSTALL_B52_HWREVISION
endif

$(eval $(autotools-package))
