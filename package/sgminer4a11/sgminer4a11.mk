################################################################################
#
# sgminer
#
################################################################################

SGMINER4A11_VERSION = 90316aba07a6b540c200ced3cf364e555fbba979
SGMINER4A11_SITE = $(call github,mineralos,sgminer4a11,$(SGMINER4A11_VERSION))
SGMINER4A11_DEPENDENCIES = host-pkgconf jansson mcompat libcurl
SGMINER4A11_AUTORECONF = YES
SGMINER4A11_CONF_ENV += LDFLAGS="$(TARGET_LDFLAGS) -lmcompat_drv "
SGMINER4A11_CONF_ENV += LIBS='-lmcompat_drv'
SGMINER4A11_CONF_OPTS = --enable-coinflex --with-system-jansson --without-curses --enable-curl

ifeq ($(BR2_INIT_SYSTEMD),y)
SGMINER4A11_CONF_OPTS += --enable-libsystemd
endif

define SGMINER4A11_INSTALL_INIT_SYSTEMD
	$(INSTALL) -D -m 0644 $(SGMINER4A11_PKGDIR)/sgminer4a11.service \
		$(TARGET_DIR)/usr/lib/systemd/system/cgminer.service
	mkdir -p $(TARGET_DIR)/etc/systemd/system/multi-user.target.wants
	ln -fs ../../../../usr/lib/systemd/system/cgminer.service \
		$(TARGET_DIR)/etc/systemd/system/multi-user.target.wants/cgminer.service
endef

define RENAME_SGMINER4A11_BIN
	mv $(TARGET_DIR)/usr/bin/sgminer $(TARGET_DIR)/usr/bin/cgminer
endef

define INSTALL_SGMINER4A11_CONFIG
	$(INSTALL) -D -m 0644 $(SGMINER4A11_PKGDIR)/cgminer.conf.default \
		$(TARGET_DIR)/etc/cgminer.conf.default
endef

define INSTALL_B29_HWREVISION
	$(INSTALL) -D -m 0644 $(SGMINER4A11_PKGDIR)/hwrevision_b29 \
		$(TARGET_DIR)/etc/hwrevision
endef

define INSTALL_D9_HWREVISION
	$(INSTALL) -D -m 0644 $(SGMINER4A11_PKGDIR)/hwrevision_d9 \
		$(TARGET_DIR)/etc/hwrevision
endef

define INSTALL_A11_CONF
	$(INSTALL) -D -m 0644 $(SGMINER4A11_PKGDIR)/b29.conf \
		$(TARGET_DIR)/etc/miner.conf
endef

define SGMINER4A11_POST_RSYNC
	cd $(SGMINER4A11_OVERRIDE_SRCDIR) && git show --pretty=format:'%H' | head -n 1 > $(@D)/cgminer_git.hash && cd -
endef

SGMINER4A11_POST_RSYNC_HOOKS += SGMINER4A11_POST_RSYNC

define INSTALL_SGMINER4A11_VERSION
        $(INSTALL) -D -m 0644 $(@D)/cgminer_git.hash \
                $(TARGET_DIR)/etc/cgminer_git.hash
endef

SGMINER4A11_POST_INSTALL_TARGET_HOOKS += RENAME_SGMINER4A11_BIN INSTALL_SGMINER4A11_CONFIG INSTALL_A11_CONF INSTALL_SGMINER4A11_VERSION

ifeq ($(BR2_PACKAGE_SGMINER_D9),y)
SGMINER4A11_POST_INSTALL_TARGET_HOOKS += INSTALL_D9_HWREVISION
else
SGMINER4A11_POST_INSTALL_TARGET_HOOKS += INSTALL_B29_HWREVISION
endif

$(eval $(autotools-package))
