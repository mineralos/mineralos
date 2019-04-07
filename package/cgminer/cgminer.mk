################################################################################
#
# cgminer
#
################################################################################

CGMINER_VERSION = e7128f35d9b5ba816113e7f959aba47c6a870a3b
CGMINER_SITE = $(call github,ckolivas,cgminer,$(CGMINER_VERSION))
CGMINER_DEPENDENCIES = host-pkgconf jansson
CGMINER_AUTORECONF = YES
CGMINER_CONF_OPTS = --enable-dragonmint_t1 --with-system-jansson --disable-libcurl
ifeq ($(BR2_INIT_SYSTEMD),y)
CGMINER_CONF_OPTS += --enable-libsystemd
endif

define CGMINER_INSTALL_INIT_SYSTEMD
	$(INSTALL) -D -m 0644 $(CGMINER_PKGDIR)/cgminer.service \
		$(TARGET_DIR)/usr/lib/systemd/system/cgminer.service
	mkdir -p $(TARGET_DIR)/etc/systemd/system/multi-user.target.wants
	ln -fs ../../../../usr/lib/systemd/system/cgminer.service \
		$(TARGET_DIR)/etc/systemd/system/multi-user.target.wants/cgminer.service
endef

define BUILD_CGMINER_API
	$(TARGET_CC) $(TARGET_CFLAGS) $(TARGET_LDFLAGS) -o $(@D)/cgminer-api \
		$(@D)/api-example.c
endef

define CGMINER_POST_RSYNC
	$(APPLY_PATCHES) $(@D) $(CGMINER_PKGDIR) \*.patch || exit 1; \
	cd $(CGMINER_OVERRIDE_SRCDIR) && git show --pretty=format:'%H' | head -n 1 > $(@D)/cgminer_git.hash && cd -
endef

define CGMINER_POST_EXTRACT
	echo "$(CGMINER_VERSION)" > $(@D)/cgminer_git.hash
endef

CGMINER_POST_BUILD_HOOKS += BUILD_CGMINER_API
CGMINER_POST_RSYNC_HOOKS += CGMINER_POST_RSYNC
CGMINER_POST_EXTRACT_HOOKS += CGMINER_POST_EXTRACT

define INSTALL_CGMINER_API
	$(INSTALL) -D -m 755 $(@D)/cgminer-api \
		$(TARGET_DIR)/usr/bin/cgminer-api
endef

define INSTALL_CGMINER_CONFIG
	$(INSTALL) -D -m 0644 $(CGMINER_PKGDIR)/cgminer.conf.default \
		$(TARGET_DIR)/etc/cgminer.conf.default
endef

define INSTALL_T1_HWREVISION
	$(INSTALL) -D -m 0644 $(CGMINER_PKGDIR)/hwrevision_t1 \
		$(TARGET_DIR)/etc/hwrevision
endef

define INSTALL_T1_CONF
	$(INSTALL) -D -m 0644 $(CGMINER_PKGDIR)/t1.conf \
		$(TARGET_DIR)/etc/miner.conf
endef

define INSTALL_T2_HWREVISION
	$(INSTALL) -D -m 0644 $(CGMINER_PKGDIR)/hwrevision_t2 \
		$(TARGET_DIR)/etc/hwrevision
endef

define INSTALL_T2_CONF
	$(INSTALL) -D -m 0644 $(CGMINER_PKGDIR)/t2.conf \
		$(TARGET_DIR)/etc/miner.conf
endef

define INSTALL_CGMINER_S9_HWREVISION
	$(INSTALL) -D -m 0644 $(CGMINER_PKGDIR)/hwrevision_s9 \
		$(TARGET_DIR)/etc/hwrevision
endef

define INSTALL_CGMINER_S9_CONF
	$(INSTALL) -D -m 0644 $(CGMINER_PKGDIR)/s9.conf \
		$(TARGET_DIR)/etc/miner.conf
endef

define INSTALL_CGMINER_VERSION
	$(INSTALL) -D -m 0644 $(@D)/cgminer_git.hash \
                $(TARGET_DIR)/etc/cgminer_git.hash
endef

CGMINER_POST_INSTALL_TARGET_HOOKS += INSTALL_CGMINER_API INSTALL_CGMINER_CONFIG INSTALL_CGMINER_VERSION

ifeq ($(BR2_PACKAGE_CGMINER_T2),y)
CGMINER_POST_INSTALL_TARGET_HOOKS += INSTALL_T2_HWREVISION INSTALL_T2_CONF
else ifeq ($(BR2_PACKAGE_CGMINER_S9),y)
CGMINER_POST_INSTALL_TARGET_HOOKS += INSTALL_CGMINER_S9_HWREVISION INSTALL_CGMINER_S9_CONF
else
CGMINER_POST_INSTALL_TARGET_HOOKS += INSTALL_T1_HWREVISION INSTALL_T1_CONF
endif

$(eval $(autotools-package))
