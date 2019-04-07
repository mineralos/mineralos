################################################################################
#
# cgminer
#
################################################################################

CGMINER4A6_VERSION = 3ad435c9a61157c22d7d3c00551479d1f265de0f
CGMINER4A6_SITE = $(call github,mineralos,cgminer4a6,$(CGMINER4A6_VERSION))
CGMINER4A6_DEPENDENCIES = host-pkgconf jansson mcompat libcurl
CGMINER4A6_AUTORECONF = YES
CGMINER4A6_CONF_ENV += LDFLAGS="$(TARGET_LDFLAGS) -lmcompat_drv "
CGMINER4A6_CONF_ENV += LIBS='-lmcompat_drv'
CGMINER4A6_CONF_OPTS = --enable-coinflex --with-system-jansson --without-curses --enable-curl

ifeq ($(BR2_INIT_SYSTEMD),y)
CGMINER4A6_CONF_OPTS += --enable-libsystemd
endif

define CGMINER4A6_INSTALL_INIT_SYSTEMD
	$(INSTALL) -D -m 0644 $(CGMINER4A6_PKGDIR)/cgminer4a6.service \
		$(TARGET_DIR)/usr/lib/systemd/system/cgminer.service
	mkdir -p $(TARGET_DIR)/etc/systemd/system/multi-user.target.wants
	ln -fs ../../../../usr/lib/systemd/system/cgminer.service \
		$(TARGET_DIR)/etc/systemd/system/multi-user.target.wants/cgminer.service
endef

define INSTALL_CGMINER4A6_CONFIG
	$(INSTALL) -D -m 0644 $(CGMINER4A6_PKGDIR)/cgminer.conf.default \
		$(TARGET_DIR)/etc/cgminer.conf.default
endef

define INSTALL_A6_HWREVISION
	$(INSTALL) -D -m 0644 $(CGMINER4A6_PKGDIR)/hwrevision_a6 \
		$(TARGET_DIR)/etc/hwrevision
endef

define INSTALL_A6_CONF
	$(INSTALL) -D -m 0644 $(CGMINER4A6_PKGDIR)/a6.conf \
		$(TARGET_DIR)/etc/miner.conf
endef

define CGMINER4A6_POST_RSYNC
	cd $(CGMINER4A6_OVERRIDE_SRCDIR) && git show --pretty=format:'%H' | head -n 1 > $(@D)/cgminer_git.hash && cd -
endef

CGMINER4A6_POST_RSYNC_HOOKS += CGMINER4A6_POST_RSYNC

define INSTALL_CGMINER4A6_VERSION
        $(INSTALL) -D -m 0644 $(@D)/cgminer_git.hash \
                $(TARGET_DIR)/etc/cgminer_git.hash
endef


CGMINER4A6_POST_INSTALL_TARGET_HOOKS += INSTALL_CGMINER4A6_CONFIG INSTALL_A6_CONF INSTALL_CGMINER4A6_VERSION INSTALL_A6_HWREVISION

$(eval $(autotools-package))
