################################################################################
#
# wg-diag
#
################################################################################

WG_DIAG_VERSION = local
WG_DIAG_SITE = $(WG_DIAG_PKGDIR)/.
WG_DIAG_SITE_METHOD = local

define WG_DIAG_INSTALL_INIT_SYSTEMD
	$(INSTALL) -D -m 0644 $(WG_DIAG_PKGDIR)/wg-diag.service \
		$(TARGET_DIR)/usr/lib/systemd/system/wg-diag.service
	mkdir -p $(TARGET_DIR)/etc/systemd/system/multi-user.target.wants
	ln -fs ../../../../usr/lib/systemd/system/wg-diag.service \
		$(TARGET_DIR)/etc/systemd/system/multi-user.target.wants/wg-diag.service
endef

$(eval $(generic-package))
