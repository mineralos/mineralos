################################################################################
#
# bmminer-mix
#
################################################################################

BMMINER_MIX_VERSION = e670d4fa138ffa5244166d8cbce6e877f3b28209
BMMINER_MIX_SITE = $(call github,ckolivas,cgminer,$(BMMINER_MIX_VERSION))
BMMINER_MIX_DEPENDENCIES = host-pkgconf jansson libcurl
BMMINER_MIX_AUTORECONF = YES
BMMINER_MIX_CONF_ENV += LDFLAGS="$(TARGET_LDFLAGS)"
BMMINER_MIX_CONF_OPTS = --enable-bitmain_soc --enable-bitmain_S9 --with-system-jansson --without-curses --enable-curl

ifeq ($(BR2_INIT_SYSTEMD),y)
BMMINER_MIX_CONF_OPTS += --enable-libsystemd
endif

define BMMINER_MIX_INSTALL_INIT_SYSTEMD
	$(INSTALL) -D -m 0644 $(BMMINER_MIX_PKGDIR)/cgminer.service \
		$(TARGET_DIR)/usr/lib/systemd/system/cgminer.service
	mkdir -p $(TARGET_DIR)/etc/systemd/system/multi-user.target.wants
	ln -fs ../../../../usr/lib/systemd/system/cgminer.service \
		$(TARGET_DIR)/etc/systemd/system/multi-user.target.wants/cgminer.service
endef

define BUILD_BMMINER_MIX_API
	$(TARGET_CC) $(TARGET_CFLAGS) $(TARGET_LDFLAGS) -o $(@D)/cgminer-api \
		$(@D)/api-example.c
endef

BMMINER_MIX_POST_BUILD_HOOKS += BUILD_BMMINER_MIX_API

define INSTALL_BMMINER_MIX_CONFIG
	$(INSTALL) -D -m 0644 $(BMMINER_MIX_PKGDIR)/cgminer.conf.default \
		$(TARGET_DIR)/etc/cgminer.conf.default
endef

define INSTALL_S9_HWREVISION
	$(INSTALL) -D -m 0644 $(BMMINER_MIX_PKGDIR)/hwrevision_s9 \
		$(TARGET_DIR)/etc/hwrevision
endef

define INSTALL_S9_CONF
	$(INSTALL) -D -m 0644 $(BMMINER_MIX_PKGDIR)/s9.conf \
		$(TARGET_DIR)/etc/miner.conf
endef

define BMMINER_MIX_POST_RSYNC
	cd $(BMMINER_MIX_OVERRIDE_SRCDIR) && git show --pretty=format:'%H' | head -n 1 > $(@D)/cgminer_git.hash && cd -
endef

define BMMINER_MIX_POST_EXTRACT
	echo "$(BMMINER_MIX_VERSION)" > $(@D)/cgminer_git.hash
endef

BMMINER_MIX_POST_RSYNC_HOOKS += BMMINER_MIX_POST_RSYNC
BMMINER_MIX_POST_EXTRACT_HOOKS += BMMINER_MIX_POST_EXTRACT

define INSTALL_BMMINER_MIX_VERSION
	$(INSTALL) -D -m 0644 $(@D)/cgminer_git.hash \
		$(TARGET_DIR)/etc/cgminer_git.hash
endef

define INSTALL_BMMINER_MIX_API
	$(INSTALL) -D -m 755 $(@D)/cgminer-api \
		$(TARGET_DIR)/usr/bin/cgminer-api
endef

define INSTALL_BMMINER_CONFIG_FILES
	mkdir -p $(TARGET_DIR)/etc/config
		$(INSTALL) -D -m 0644 $(BMMINER_MIX_PKGDIR)/config/* $(TARGET_DIR)/etc/config/
	mkdir -p $(TARGET_DIR)/etc/config/minertest64
		$(INSTALL) -D -m 0644 $(BMMINER_MIX_PKGDIR)/minertest64/* $(TARGET_DIR)/etc/config/minertest64/
		$(INSTALL) -D -m 0644 $(BMMINER_MIX_PKGDIR)/ctrl_bd $(TARGET_DIR)/usr/bin/ctrl_bd
	ln -fs ../../config/restartTest \
		$(TARGET_DIR)/etc/config/restartTest
	ln -fs ../../config/rebootTest \
		$(TARGET_DIR)/etc/config/rebootTest
	ln -fs ../../config/retryFlag \
		$(TARGET_DIR)/etc/config/retryFlag
	ln -fs ../../config/testID \
		$(TARGET_DIR)/etc/config/testID
	ln -fs ../../config/lastlog \
		$(TARGET_DIR)/etc/config/lastlog
endef

BMMINER_MIX_POST_INSTALL_TARGET_HOOKS += INSTALL_BMMINER_MIX_API INSTALL_BMMINER_MIX_CONFIG INSTALL_S9_CONF INSTALL_BMMINER_MIX_VERSION INSTALL_BMMINER_CONFIG_FILES

ifeq ($(BR2_PACKAGE_BMMINER_MIX_S9),y)
BMMINER_MIX_POST_INSTALL_TARGET_HOOKS += INSTALL_S9_HWREVISION
endif

$(eval $(autotools-package))
