############################ 引用目录 ########################################
-include ../dirs.mk
include  ../config.mk
include  ../toolchain.mk

# 编译参数
CFLAGS  += -fPIC -o
LDFLAGS += -shared -lpthread
##############################################################################

############################### 目录 ##########################################
# 通用目录
MODULE_PATH  = $(SRC_DIR)/module
DRV_PATH     = $(SRC_DIR)/drv
ZYNQ_PATH    = $(SRC_DIR)/zynq
OPI_PATH     = $(SRC_DIR)/orangepi
HUB_PATH     = $(SRC_DIR)/hub
MISC_PATH    = $(SRC_DIR)/misc
SYS_PATH     = $(SRC_DIR)/platform
VPATH        = $(MODULE_PATH) $(DRV_PATH) $(ZYNQ_PATH) $(OPI_PATH) \
               $(HUB_PATH) $(SYS_PATH) $(MISC_PATH)
##############################################################################

############################## C编文件 #######################################

# 需要编译的c文件
MODULE_C_FILES = mcompat_cmd.c mcompat_gpio.c mcompat_pwm.c mcompat_fan.c mcompat_chain.c mcompat_log.c mcompat_watchdog.c mcompat_tvsensor.c
DRV_C_FILES    = util.c drv_hub.c drv_spi.c drv_opi.c drv_zynq.c hub_cmd.c spi_cmd.c opi_cmd.c
ZYNQ_C_FILES   = zynq_gpio.c zynq_spi.c zynq_vid.c zynq_pwm.c
OPI_C_FILFS    = opi_h3.c opi_spi.c
HUB_C_FILES    = hub_vid.c
SYS_C_FILES    = platform.c
MISC_C_FILES   = mcompat_misc.c
C_FILES        = $(MODULE_C_FILES) $(DRV_C_FILES) $(ZYNQ_C_FILES) \
		 $(OPI_C_FILFS) $(HUB_C_FILES) $(SYS_C_FILES) \
		 $(MISC_C_FILES)
##############################################################################

############################# inc目录 #########################################
INCLUDE += -I$(SRC_DIR) -I$(MODULE_PATH) -I$(DRV_PATH) -I$(ZYNQ_PATH) \
	   -I$(OPI_PATH) -I$(SYS_PATH) -I$(MISC_PATH) -I$(HUB_PATH)
##############################################################################

########################### 构建依赖关系 ##################################### 
OBJ     = $(patsubst %.c,$(OBJ_DIR)/%.o,$(C_FILES))
LINT_F  = $(patsubst %.c,$(OBJ_DIR)/%.lint,$(C_FILES))
##############################################################################

############################### 目标 ########################################
TARGET=libmcompat_drv.so
##############################################################################

all: $(TARGET)

install:
	@echo [Install... $(TARGET)]
	$(QUIET)$(CP) $(TARGET) $(RELEASE_LIB_DIR)
	$(CP) $(SYS_PATH)/mcompat_drv.h $(RELEASE_INC_DIR)
	$(CP) $(MISC_PATH)/mcompat_lib.h $(RELEASE_INC_DIR)
	$(CP) $(MODULE_PATH)/mcompat_fan.h $(RELEASE_INC_DIR)
	$(CP) $(MODULE_PATH)/mcompat_temp.h $(RELEASE_INC_DIR)
	$(CP) $(MODULE_PATH)/mcompat_log.h $(RELEASE_INC_DIR)
	$(CP) $(SRC_DIR)/mcompat_config.h $(RELEASE_INC_DIR)

-include ../common.mk
