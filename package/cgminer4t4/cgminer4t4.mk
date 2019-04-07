################################################################################
#
# cgminer
#
################################################################################

CGMINER4T4_VERSION = 89c66c5923cc34fa789ba3c00a6b1b7e5d9d8ba5
CGMINER4T4_SITE = $(call github,mineralos,cgminer4t4,$(CGMINER4T4_VERSION))
CGMINER4T4_DEPENDENCIES = host-pkgconf jansson mcompat
CGMINER4T4_AUTORECONF = YES
CGMINER4T4_CONF_ENV += LDFLAGS="$(TARGET_LDFLAGS) -lmcompat_drv "
CGMINER4T4_CONF_ENV += LIBS='-lmcompat_drv'
CGMINER4T4_CONF_OPTS = --enable-bitmine_t4 --with-system-jansson --disable-libcurl
ifeq ($(BR2_INIT_SYSTEMD),y)
CGMINER4T4_CONF_OPTS += --enable-libsystemd
endif

define CGMINER4T4_INSTALL_INIT_SYSTEMD
	$(INSTALL) -D -m 0644 $(CGMINER4T4_PKGDIR)/cgminer4t4.service \
		$(TARGET_DIR)/usr/lib/systemd/system/cgminer.service
	mkdir -p $(TARGET_DIR)/etc/systemd/system/multi-user.target.wants
	ln -fs ../../../../usr/lib/systemd/system/cgminer.service \
		$(TARGET_DIR)/etc/systemd/system/multi-user.target.wants/cgminer.service
endef

define BUILD_CGMINER4T4_API
	$(TARGET_CC) $(TARGET_CFLAGS) $(TARGET_LDFLAGS) -o $(@D)/cgminer-api \
		$(@D)/api-example.c
endef

define CGMINER4T4_POST_RSYNC
	cd $(CGMINER4T4_OVERRIDE_SRCDIR) && git show --pretty=format:'%H' | head -n 1 > $(@D)/cgminer_git.hash && cd -
endef

CGMINER4T4_POST_BUILD_HOOKS += BUILD_CGMINER4T4_API 
CGMINER4T4_POST_RSYNC_HOOKS += CGMINER4T4_POST_RSYNC

define INSTALL_CGMINER4T4_API
	$(INSTALL) -D -m 755 $(@D)/cgminer-api \
		$(TARGET_DIR)/usr/bin/cgminer-api
endef

define INSTALL_CGMINER4T4_CONFIG
	$(INSTALL) -D -m 0644 $(CGMINER4T4_PKGDIR)/cgminer.conf.default \
		$(TARGET_DIR)/etc/cgminer.conf.default
endef

define INSTALL_T4_HWREVISION
	$(INSTALL) -D -m 0644 $(CGMINER4T4_PKGDIR)/hwrevision_a8plus \
		$(TARGET_DIR)/etc/hwrevision
endef

define INSTALL_T4_CONF
	$(INSTALL) -D -m 0644 $(CGMINER4T4_PKGDIR)/a8plus.conf \
		$(TARGET_DIR)/etc/miner.conf
endef

define INSTALL_CGMINER4T4_VERSION
	$(INSTALL) -D -m 0644 $(@D)/cgminer_git.hash \
                $(TARGET_DIR)/etc/cgminer_git.hash
endef

CGMINER4T4_POST_INSTALL_TARGET_HOOKS += INSTALL_CGMINER4T4_API INSTALL_CGMINER4T4_CONFIG INSTALL_T4_HWREVISION INSTALL_T4_CONF INSTALL_CGMINER4T4_VERSION

$(eval $(autotools-package))
