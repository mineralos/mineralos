################################################################################
#
# zynq-mkbootimage
#
################################################################################

ZYNQ_MKBOOTIMAGE_VERSION = d9d4e6f8ca3f0560560d163760314d938f58a0ba
ZYNQ_MKBOOTIMAGE_NAME:=mkbootimage
ZYNQ_MKBOOTIMAGE_VERSION_MAJOR:=2.1
ZYNQ_MKBOOTIMAGE_VERSION_MINOR:=d9d4e6f
ZYNQ_MKBOOTIMAGE_SITE = $(call github,antmicro,zynq-mkbootimage,$(ZYNQ_MKBOOTIMAGE_VERSION))
ZYNQ_MKBOOTIMAGE_VER:=${ZYNQ_MKBOOTIMAGE_NAME} ${ZYNQ_MKBOOTIMAGE_VERSION_MAJOR}-${ZYNQ_MKBOOTIMAGE_VERSION_MINOR}

HOST_ZYNQ_MKBOOTIMAGE_DEPENDENCIES = host-elfutils host-pcre
ZYNQ_MKBOOTIMAGE_CFLAGS = \
	-I$(HOST_ZYNQ_MKBOOTIMAGE_DIR)/src

define ZYNQ_MKBOOTIMAGE_PRE_CONFIGURE_FIXUP
	$(SED) "s/MKBOOTIMAGE_VER/\"${ZYNQ_MKBOOTIMAGE_VER}\"/g;" $(@D)/src/mkbootimage.c
endef

define HOST_ZYNQ_MKBOOTIMAGE_BUILD_CMDS
	$(ZYNQ_MKBOOTIMAGE_PRE_CONFIGURE_FIXUP)
	$(HOST_MAKE_ENV) $(MAKE) $(HOST_CONFIGURE_OPTS) \
	CFLAGS+='$(ZYNQ_MKBOOTIMAGE_CFLAGS)' -C $(@D)
endef

define HOST_ZYNQ_MKBOOTIMAGE_INSTALL_CMDS
	cp -f $(@D)/mkbootimage $(HOST_DIR)/bin
endef

$(eval $(host-generic-package))
