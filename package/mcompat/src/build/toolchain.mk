-include config.mk

# 编译变量
CC      = $(TARGET_CROSS)gcc
LD      = $(TARGET_CROSS)gcc
READELF = $(TARGET_CROSS)readelf -a
OBJDUMP = $(TARGET_CROSS)objdump -thDS
STRIP   = $(TARGET_CROSS)strip
CP      = cp
RM      = rm -rf
GREP    = egrep -v
LINT    = splint

LINT_FLAGS = +posixlib -preproc
CFLAGS  = $(TARGET_CFLAGS)
LDFLAGS = $(TARGET_LDFLAGS)
CFLAGS  += -Wall -c
LDFLAGS += -Wall

