# This file is part of buildroot-submodule.
#
#    buildroot-submodule is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    buildroot-submodule is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with buildroot-submodule.  If not, see <http://www.gnu.org/licenses/>. 2

#location of the buildroot sources
MAKEARGS := -C $(CURDIR)/buildroot
#location to store build files
MAKEARGS += O=$(CURDIR)/$(PROJECT_NAME)/output
# location to store extra config options and buildroot packages
MAKEARGS += BR2_EXTERNAL=$(CURDIR)
#transmit project name to be able to use it in kconfig
MAKEARGS += PROJECT_NAME=$(PROJECT_NAME)
# location of default defconfig
BR2_EXTERNAL_BUILDROOT_SUBMODULE_PATH=${CURDIR}
MAKEARGS += BR2_DL_DIR="${BR2_EXTERNAL_BUILDROOT_SUBMODULE_PATH}/dl"
MAKEARGS += BR2_ROOTFS_OVERLAY="${BR2_EXTERNAL_BUILDROOT_SUBMODULE_PATH}/overlay"
MAKEARGS += BR2_PACKAGE_OVERRIDE_FILE="${BR2_EXTERNAL_BUILDROOT_SUBMODULE_PATH}/local.mk"
MAKEARGS += BR2_GLOBAL_PATCH_DIR="${BR2_EXTERNAL_BUILDROOT_SUBMODULE_PATH}/patch"
DEFCONFIG_FILE=$(CURDIR)/$(PROJECT_NAME)/defconfig
DEFCONFIG := BR2_DEFCONFIG=$(DEFCONFIG_FILE)
ALT_DEFCONFIG := BR2_DEFCONFIG=$(CURDIR)/defconfig

MAKEFLAGS += --no-print-directory

#these targets change the config file
config_change_targets:=menuconfig nconfig xconfig gconfig oldconfig \
       	silentoldconfig randconfig allyesconfig allnoconfig randpackageconfig \
       	allyespackageconfig allnopackageconfig

special_target:=$(config_change_targets) Makefile clean distclean defconfig \
	savedefconfig %_defconfig swupdate-menuconfig uboot-menuconfig \
	cgminer-% dm-monitor-% php-% swupdate-ext-menuconfig \
	cgminer4t4-% sgminer4a11-% sgminer4a12-% mcompat-% dm-selftest-% \
	dm-frontend-% dm-backend-% im-frontend-% im-backend-% bmminer-mix-% \
	ckpool-%

all	:= $(filter-out $(special_target),$(MAKECMDGOALS))

default:
	git submodule update --init --recursive
	$(MAKE) $(MAKEARGS) $(DEFCONFIG) defconfig
	$(MAKE) $(MAKEARGS) $(DEFCONFIG)

.PHONY: $(special_target) $(all)

clean:
	$(MAKE) $(MAKEARGS) $(DEFCONFIG) clean

distclean:
	$(MAKE) $(MAKEARGS) $(DEFCONFIG) distclean

# update from current config and save it as defconfig
defconfig:
	$(MAKE) $(MAKEARGS) $(ALT_DEFCONFIG) $@
	$(MAKE) $(MAKEARGS) $(DEFCONFIG) savedefconfig

# update from defconfig and save it as current configuration
savedefconfig:
	$(MAKE) $(MAKEARGS) $(DEFCONFIG) defconfig
	$(MAKE) $(MAKEARGS) $(ALT_DEFCONFIG) savedefconfig

swupdate-menuconfig:
	$(MAKE) $(MAKEARGS) $(DEFCONFIG) defconfig
	$(MAKE) $(MAKEARGS) $(DEFCONFIG) swupdate-menuconfig

swupdate-ext-menuconfig:
	$(MAKE) $(MAKEARGS) $(DEFCONFIG) defconfig
	$(MAKE) $(MAKEARGS) $(DEFCONFIG) swupdate-ext-menuconfig

uboot-menuconfig:
	$(MAKE) $(MAKEARGS) $(DEFCONFIG) defconfig
	$(MAKE) $(MAKEARGS) $(DEFCONFIG) MENUCONFIG_COLOR=mono uboot-menuconfig

cgminer-%:
	$(MAKE) $(MAKEARGS) $(DEFCONFIG) defconfig
	$(MAKE) $(MAKEARGS) $(DEFCONFIG) $@

cgminer4t4-%:
	$(MAKE) $(MAKEARGS) $(DEFCONFIG) defconfig
	$(MAKE) $(MAKEARGS) $(DEFCONFIG) $@

sgminer4a11-%:
	$(MAKE) $(MAKEARGS) $(DEFCONFIG) defconfig
	$(MAKE) $(MAKEARGS) $(DEFCONFIG) $@

sgminer4a12-%:
	$(MAKE) $(MAKEARGS) $(DEFCONFIG) defconfig
	$(MAKE) $(MAKEARGS) $(DEFCONFIG) $@

bmminer-mix-%:
	$(MAKE) $(MAKEARGS) $(DEFCONFIG) defconfig
	$(MAKE) $(MAKEARGS) $(DEFCONFIG) $@

ckpool-%:
	$(MAKE) $(MAKEARGS) $(DEFCONFIG) defconfig
	$(MAKE) $(MAKEARGS) $(DEFCONFIG) $@

php-%:
	$(MAKE) $(MAKEARGS) $(DEFCONFIG) defconfig
	$(MAKE) $(MAKEARGS) $(DEFCONFIG) $@

mcompat-%:
	$(MAKE) $(MAKEARGS) $(DEFCONFIG) defconfig
	$(MAKE) $(MAKEARGS) $(DEFCONFIG) $@

dm-monitor-%:
	$(MAKE) $(MAKEARGS) $(DEFCONFIG) defconfig
	$(MAKE) $(MAKEARGS) $(DEFCONFIG) $@

dm-selftest-%:
	$(MAKE) $(MAKEARGS) $(DEFCONFIG) defconfig
	$(MAKE) $(MAKEARGS) $(DEFCONFIG) $@

dm-frontend-%:
	$(MAKE) $(MAKEARGS) $(DEFCONFIG) defconfig
	$(MAKE) $(MAKEARGS) $(DEFCONFIG) $@

dm-backend-%:
	$(MAKE) $(MAKEARGS) $(DEFCONFIG) defconfig
	$(MAKE) $(MAKEARGS) $(DEFCONFIG) $@

im-frontend-%:
	$(MAKE) $(MAKEARGS) $(DEFCONFIG) defconfig
	$(MAKE) $(MAKEARGS) $(DEFCONFIG) $@

im-backend-%:
	$(MAKE) $(MAKEARGS) $(DEFCONFIG) defconfig
	$(MAKE) $(MAKEARGS) $(DEFCONFIG) $@

# generate from a defconfig then save as current configuration
%_defconfig:
	$(MAKE) $(MAKEARGS) $(DEFCONFIG) $@
	$(MAKE) $(MAKEARGS) $(DEFCONFIG) savedefconfig
	$(call UPDATE_DEFCONFIG)


# update from current configuration, run the command, then save the result
$(config_change_targets): $(DEFCONFIG_FILE)
	$(MAKE) $(MAKEARGS) $(DEFCONFIG) defconfig $@
	$(MAKE) $(MAKEARGS) $(DEFCONFIG) savedefconfig

_all:
	$(MAKE) $(MAKEARGS) $(DEFCONFIG)

$(all): _all
	@:

%/: _all
	@:

Makefile:;

$(DEFCONFIG_FILE):
	$(call UPDATE_DEFCONFIG)

define UPDATE_DEFCONFIG
	echo 'BR2_DL_DIR="$$(BR2_EXTERNAL_BUILDROOT_SUBMODULE_PATH)/dl"' >> $(DEFCONFIG_FILE)
	echo 'BR2_ROOTFS_OVERLAY="$$(BR2_EXTERNAL_BUILDROOT_SUBMODULE_PATH)/overlay"' >> $(DEFCONFIG_FILE)
	echo 'BR2_PACKAGE_OVERRIDE_FILE="$$(BR2_EXTERNAL_BUILDROOT_SUBMODULE_PATH)/local.mk"' >> $(DEFCONFIG_FILE)
	echo 'BR2_GLOBAL_PATCH_DIR="$$(BR2_EXTERNAL_BUILDROOT_SUBMODULE_PATH)/patch"' >> $(DEFCONFIG_FILE)
	$(MAKE) $(MAKEARGS) $(DEFCONFIG) defconfig
	$(MAKE) $(MAKEARGS) $(DEFCONFIG) savedefconfig
endef


