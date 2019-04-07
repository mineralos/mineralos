include config.mk
include toolchain.mk

BUILD_DIR       = $(shell pwd)
ROOT_DIR        = $(BUILD_DIR)/..
SRC_DIR         = $(ROOT_DIR)/src
RELEASE_DIR     = $(ROOT_DIR)/release
RELEASE_INC_DIR =$(RELEASE_DIR)/include
RELEASE_BIN_DIR =$(RELEASE_DIR)/bin
RELEASE_LIB_DIR =$(RELEASE_DIR)/lib
HOST_TOOLS_DIR  = $(ROOT_DIR)/host_tools
OBJ_DIR         = objs

all: install

# 生成目录文件,供整个makefile系统使用(便于不同机器的移植)
prepare:
	@echo
	@echo "============================== Prepare ============================="
	$(QUIET)mkdir -p $(RELEASE_INC_DIR)
	$(QUIET)mkdir -p $(RELEASE_LIB_DIR)
	$(QUIET)mkdir -p $(RELEASE_BIN_DIR)
	$(QUIET)mkdir -p $(HOST_TOOLS_DIR)
	$(QUIET)echo "ROOT_DIR        = $(ROOT_DIR)" > dirs.mk
	$(QUIET)echo "SRC_DIR         = $(SRC_DIR)" >> dirs.mk
	$(QUIET)echo "RELEASE_DIR     = $(RELEASE_DIR)" >> dirs.mk
	$(QUIET)echo "RELEASE_INC_DIR = $(RELEASE_INC_DIR)" >> dirs.mk
	$(QUIET)echo "RELEASE_BIN_DIR = $(RELEASE_BIN_DIR)" >> dirs.mk
	$(QUIET)echo "RELEASE_LIB_DIR = $(RELEASE_LIB_DIR)" >> dirs.mk
	$(QUIET)echo "HOST_TOOLS_DIR  = $(RELEASE_LIB_DIR)" >> dirs.mk
	$(QUIET)echo "BUILD_DIR       = $(BUILD_DIR)" >> dirs.mk
	$(QUIET)echo "OBJ_DIR         = $(OBJ_DIR)" >> dirs.mk
	@echo "============================== Prepare End ========================="
	@echo

# 主编译过程
build: prepare
	@echo
	@echo "=============================== Build =============================="
	$(QUIET)$(MAKE) -C mcompat_drv all $(SILENCE)
	$(QUIET)$(MAKE) -C dm_test all $(SILENCE)
	$(QUIET)$(MAKE) -C dm_chip_type all $(SILENCE)
	@echo "============================= Build End ============================"
	@echo

install: build
	@echo
	@echo "============================== Install ============================="
	$(QUIET)$(MAKE) -C mcompat_drv install $(SILENCE)
	$(QUIET)$(MAKE) -C dm_test install $(SILENCE)
	$(QUIET)$(MAKE) -C dm_chip_type install $(SILENCE)
	@echo "============================ Install End ==========================="

clean: prepare
	$(QUIET)$(MAKE) -C dm_chip_type clean $(SILENCE)
	$(QUIET)$(MAKE) -C dm_test clean $(SILENCE)
	$(QUIET)$(MAKE) -C mcompat_drv clean $(SILENCE)
	$(QUIET)$(RM) $(RELEASE_DIR)
	$(QUIET)$(RM) $(HOST_TOOLS_DIR)
	$(QUIET)$(RM) $(BUILD_DIR)/dirs.mk

.PHONY: prepare clean

