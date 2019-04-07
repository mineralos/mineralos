################################################################################
#
# sgminer
#
################################################################################

SGMINER4A7_VERSION = 95570c46fc139e3ba1841807ad4319f8e4e101d1
SGMINER4A7_SITE = $(call github,mineralos,sgminer4a7,$(SGMINER4A7_VERSION))
SGMINER4A7_DEPENDENCIES = host-pkgconf jansson mcompat libcurl
SGMINER4A7_AUTORECONF = YES
SGMINER4A7_CONF_ENV += LDFLAGS="$(TARGET_LDFLAGS) -lmcompat_drv "
SGMINER4A7_CONF_ENV += LIBS='-lmcompat_drv'
SGMINER4A7_CONF_OPTS = --enable-coinflex --with-system-jansson --without-curses --enable-curl

ifeq ($(BR2_INIT_SYSTEMD),y)
SGMINER4A7_CONF_OPTS += --enable-libsystemd
endif

define SGMINER4A7_INSTALL_INIT_SYSTEMD
	$(INSTALL) -D -m 0644 $(SGMINER4A7_PKGDIR)/sgminer4a7.service \
		$(TARGET_DIR)/usr/lib/systemd/system/cgminer.service
	mkdir -p $(TARGET_DIR)/etc/systemd/system/multi-user.target.wants
	ln -fs ../../../../usr/lib/systemd/system/cgminer.service \
		$(TARGET_DIR)/etc/systemd/system/multi-user.target.wants/cgminer.service
endef

define RENAME_SGMINER4A7_BIN
	mv $(TARGET_DIR)/usr/bin/sgminer $(TARGET_DIR)/usr/bin/cgminer
endef

define INSTALL_SGMINER4A7_CONFIG
	$(INSTALL) -D -m 0644 $(SGMINER4A7_PKGDIR)/cgminer.conf.default \
		$(TARGET_DIR)/etc/cgminer.conf.default
endef

define INSTALL_A5_HWREVISION
	$(INSTALL) -D -m 0644 $(SGMINER4A7_PKGDIR)/hwrevision_a5 \
		$(TARGET_DIR)/etc/hwrevision
endef

define INSTALL_A5_CONF
	$(INSTALL) -D -m 0644 $(SGMINER4A7_PKGDIR)/a5.conf \
		$(TARGET_DIR)/etc/miner.conf
endef

define SGMINER4A7_POST_RSYNC
	cd $(SGMINER4A7_OVERRIDE_SRCDIR) && git show --pretty=format:'%H' | head -n 1 > $(@D)/cgminer_git.hash && cd -
endef

SGMINER4A7_POST_RSYNC_HOOKS += SGMINER4A7_POST_RSYNC

define INSTALL_SGMINER4A7_VERSION
        $(INSTALL) -D -m 0644 $(@D)/cgminer_git.hash \
                $(TARGET_DIR)/etc/cgminer_git.hash
endef


SGMINER4A7_POST_INSTALL_TARGET_HOOKS += RENAME_SGMINER4A7_BIN INSTALL_SGMINER4A7_CONFIG INSTALL_A5_CONF INSTALL_SGMINER4A7_VERSION INSTALL_A5_HWREVISION

$(eval $(autotools-package))
