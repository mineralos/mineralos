################################################################################
#
# miner compatibility layer code
#
################################################################################

MCOMPAT_VERSION = local
MCOMPAT_SITE = $(MCOMPAT_PKGDIR)/src
MCOMPAT_SITE_METHOD = local
MCOMPAT_INSTALL_STAGING = YES
MCOMPAT_INSTALL_TARGET = YES
MCOMPAT_MODULES = module drv zynq orangepi hub misc platform
MCOMPAT_LIB_INCLUDE += $(foreach include,$(dir $(wildcard $(@D)/src/*/)),-I$(patsubst %/,%,$(include))) -I$(@D)/src


define MCOMPAT_BUILD_CMDS
	$(foreach module, $(MCOMPAT_MODULES), \
		cd $(@D)/src/$(module); \
		$(TARGET_CC) $(MCOMPAT_LIB_INCLUDE) $(TARGET_CFLAGS) -c *.c
	)
	cd $(@D)/src; $(TARGET_AR) rcu libmcompat_drv.a ./*/*.o
endef

define MCOMPAT_INSTALL_STAGING_CMDS
	$(INSTALL) -m 644 -D $(@D)/src/*.h              $(STAGING_DIR)/usr/include/
	$(INSTALL) -m 644 -D $(@D)/src/*/*.h            $(STAGING_DIR)/usr/include/
	$(INSTALL) -m 644 -D $(@D)/src/libmcompat_drv.a      $(STAGING_DIR)/usr/lib/libmcompat_drv.a
endef

define BUILD_DM_TEST
	$(TARGET_CC) $(TARGET_CFLAGS) $(TARGET_LDFLAGS) \
		-lpthread -o $(@D)/src/dm_test \
		$(@D)/src/app/dm_test.c -lmcompat_drv
endef

define BUILD_DM_CHIP_TYPE
	$(TARGET_CC) $(TARGET_CFLAGS) $(TARGET_LDFLAGS) \
		-lpthread -o $(@D)/src/dm_chip_type \
		$(@D)/src/app/dm_chip_type.c -lmcompat_drv
endef

MCOMPAT_POST_INSTALL_STAGING_HOOKS += BUILD_DM_TEST BUILD_DM_CHIP_TYPE

define MCOMPAT_INSTALL_TARGET_CMDS
	$(INSTALL) -m 755 -D $(@D)/src/dm_test          $(TARGET_DIR)/usr/bin/dm_test
	$(INSTALL) -m 755 -D $(@D)/src/dm_chip_type     $(TARGET_DIR)/usr/bin/dm_chip_type
endef

$(eval $(generic-package))
