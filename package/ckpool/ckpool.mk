################################################################################
#
# ckpool
#
################################################################################

CKPOOL_VERSION = 5d4dbe166c31333889aed58bfb32d1f49e520d26
CKPOOL_SITE = https://bitbucket.org/ckolivas/ckpool/downloads
CKPOOL_DEPENDENCIES = jansson
CKPOOL_SITE_METHOD = git
CKPOOL_AUTORECONF = YES

define CKPOOL_INSTALL_INIT_SYSTEMD
	$(INSTALL) -D -m 0644 $(CKPOOL_PKGDIR)/ckpool.service \
		$(TARGET_DIR)/usr/lib/systemd/system/ckpool.service
	mkdir -p $(TARGET_DIR)/etc/systemd/system/multi-user.target.wants
	ln -fs ../../../../usr/lib/systemd/system/ckpool.service \
		$(TARGET_DIR)/etc/systemd/system/multi-user.target.wants/ckpool.service
endef

define INSTALL_CKPOOL_CONFIG
	$(INSTALL) -D -m 0644 $(CKPOOL_PKGDIR)/ckpool.conf.default \
		$(TARGET_DIR)/etc/ckpool.conf.default
endef

CKPOOL_POST_INSTALL_TARGET_HOOKS += INSTALL_CKPOOL_CONFIG

$(eval $(autotools-package))
