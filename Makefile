################################################################################
# Makefile for pilcd2004tty project
#
# Hardware: Raspberry Pi 5 (aarch64)
# Peripherals:
#   LCD2004 LCD screen with i2c interface
#   USB-to-UART bridge (LPC231x)
#
# @copyright 2026 Purple Monkey Software. All rights reserved.
################################################################################

# Project
PROJECT = pilcd2004tty

#enable/disable features
# Enable this to use the debug UART loopback feature
DEBUG_UART_LOOPBACK  = disable
# Enable this to debug I/O
DEBUG_IO             = disable

# platform
PLATFORM = raspberrypi

# build directory structure
BUILD_DIR   = ./build
TARGET      = $(BUILD_DIR)/$(PROJECT)

# Source files
C_SRCS         = $(shell find src -name "*.c")
C_SRCS        += $(shell find dev -name "*.c")
C_SRCS        += $(shell find monkeyHAL/common -name "*.c")
C_SRCS        += $(shell find monkeyHAL/linux -name "*i2c.c")
C_SRCS        += $(shell find monkeyHAL/linux -name "*timer.c")
C_SRCS        += $(shell find monkeyHAL/linux -name "*uart.c")
C_OBJLIST      = $(addprefix $(BUILD_DIR)/, $(C_SRCS:.c=.o))
C_DEPLIST      = $(C_OBJLIST:.o=.d)

# Include directories
INCLUDES  = -I./src
INCLUDES += -I./dev
INCLUDES += -I./monkeyHAL/common
INCLUDES += -I./monkeyHAL/linux

# Toolchain
CROSS_COMPILE = aarch64-linux-gnu-
CC      = $(CROSS_COMPILE)gcc
MKDIR   = mkdir -p
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump
RM      = rm
SIZE    = $(CROSS_COMPILE)size
STRIP   = $(CROSS_COMPILE)strip

# Toolchain check: make sure the compiler exists and targets aarch64 before
# compiling anything.  Skipped for "make clean".
TARGET_ARCH_EXPECTED = aarch64
ifneq ($(filter-out clean,$(or $(MAKECMDGOALS),all)),)
CC_PATH    := $(shell command -v $(CC) 2>/dev/null)
ifeq ($(CC_PATH),)
$(error Compiler '$(CC)' not found. Install it with: sudo apt install gcc-aarch64-linux-gnu binutils-aarch64-linux-gnu)
endif
CC_MACHINE := $(shell $(CC) -dumpmachine)
ifeq ($(findstring $(TARGET_ARCH_EXPECTED),$(CC_MACHINE)),)
$(error Compiler '$(CC)' targets '$(CC_MACHINE)', expected $(TARGET_ARCH_EXPECTED). Check CROSS_COMPILE)
endif
endif

# compiler flags
C_FLAGS = -Wall -Wextra -O2 -g -Wno-unused-function -pthread
ifeq ($(strip $(DEBUG_UART_LOOPBACK)),enable)
C_FLAGS += -DDEBUG_UART_LOOPBACK
endif
ifeq ($(strip $(DEBUG_IO)),enable)
C_FLAGS += -DDEBUG_IO
endif

# platform flags
ifeq ($(strip $(PLATFORM)),raspberrypi)
C_FLAGS += -DRASPBERRY_PI
endif

# Targets
all: $(TARGET) size-output

# build project object code
# -MMD -MP generate a .d file next to each object listing the headers it
# includes, so header changes trigger a rebuild of the affected objects.
$(BUILD_DIR)/%.o: %.c
	@echo compiling $<
	@$(MKDIR) $(dir $@)
	@$(CC) $(INCLUDES) $(C_FLAGS) -MMD -MP -c -o $@ $<

# pull in auto-generated header dependencies (missing on first build is OK)
-include $(C_DEPLIST)

# build target (only relinks when an object file changes)
$(TARGET): $(C_OBJLIST)
	@echo ' '
	@echo "linking target and map file..."
	@$(CC) -pthread $(C_OBJLIST) -Wl,-Map=$(BUILD_DIR)/$(PROJECT).map -o $@
	@echo 'Finished building target: $@'
	@echo "Build debug symbols"
	@$(OBJCOPY) --only-keep-debug $@ $@.debug
	@$(OBJCOPY) --strip-debug $@
	@$(STRIP) --strip-unneeded $@
	@$(OBJCOPY) --add-gnu-debuglink=$@.debug $@
	@echo "Build assembly listing"
	@$(OBJDUMP) -hS $@ > $(BUILD_DIR)/$(PROJECT).lst
	@echo ' '

# display build stats
size-output: $(TARGET)
	@$(SIZE)  $(TARGET)
	@echo ' '
	@echo "Build complete"

clean:
	-$(RM) -fr $(BUILD_DIR)
	-@echo ' '

.PHONY: all size-output clean
