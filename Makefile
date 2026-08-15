# ============================================================================
# Makefile for ESP8266 GUIKit Project
# 
# Provides convenient targets for building the GUIKit system
# 
# USAGE:
#   make [target] [options]
# 
# TARGETS:
#   all           Build everything (default)
#   bootloader    Build bootloader
#   kernel        Build kernel
#   sdcard        Prepare SD card structure
#   gui           Build GUI projects
#   clean         Clean all build artifacts
#   flash         Flash bootloader and kernel
#   test          Run tests
#   help          Show this help
#
# OPTIONS:
#   VERBOSE=1     Enable verbose output
#   UPLOAD=1      Upload after build
#   PORT=/dev/ttyUSB0  Specify serial port
#   SDCARD=/path/to/sd   Specify SD card path
#
# EXAMPLES:
#   make all                     # Build everything
#   make bootloader UPLOAD=1     # Build and upload bootloader
#   make sdcard SDCARD=/Volumes/SDCARD  # Use real SD card
#   make clean                    # Clean all artifacts
#   make -j4                      # Parallel build (where supported)
#
# DEPENDENCIES:
#   - PlatformIO CLI
#   - GNU Make
#   - Bash shell
# ============================================================================

# Configuration
BUILD_SCRIPT := ./build.sh
SDK_PATH ?= ./sdcard
BOOTLOADER_ENV ?= bootloader
KERNEL_ENV ?= kernel

# Default target
.PHONY: default all

# ============================================================================
# Help System
# ============================================================================

.PHONY: help
help:
	@echo "ESP8266 GUIKit Makefile"
	@echo ""
	@echo "USAGE:"
	@echo "  make [target] [VAR=value]"
	@echo ""
	@echo "TARGETS:"
	@echo "  all           Build everything (bootloader, kernel, SD card, GUIs)"
	@echo "  bootloader    Build bootloader only"
	@echo "  kernel        Build kernel only"
	@echo "  sdcard        Prepare SD card structure only"
	@echo "  gui           Build GUI projects only"
	@echo "  clean         Clean all build artifacts"
	@echo "  flash         Flash bootloader to device"
	@echo "  test          Run tests"
	@echo "  help          Show this help message"
	@echo ""
	@echo "OPTIONS:"
	@echo "  VERBOSE=1     Enable verbose output"
	@echo "  UPLOAD=1      Upload after build"
	@echo "  PORT=path     Specify serial port (e.g., /dev/ttyUSB0)"
	@echo "  SDCARD=path   Specify SD card mount point"
	@echo ""
	@echo "EXAMPLES:"
	@echo "  make all"
	@echo "  make bootloader UPLOAD=1 PORT=/dev/ttyUSB0"
	@echo "  make sdcard SDCARD=/Volumes/SDCARD"
	@echo "  make clean"

# ============================================================================
# Version
# ============================================================================

.PHONY: version
version:
	@./build.sh version

# ============================================================================
# Build Targets
# ============================================================================

.PHONY: all
all: clean bootloader kernel sdcard

.PHONY: bootloader
bootloader:
	@echo "Building bootloader..."
	$(BUILD_SCRIPT) bootloader $(OPTS)

.PHONY: kernel
kernel:
	@echo "Building kernel..."
	$(BUILD_SCRIPT) kernel $(OPTS)

.PHONY: sdcard
sdcard:
	@echo "Preparing SD card..."
	$(BUILD_SCRIPT) sdcard $(OPTS)

.PHONY: gui
gui:
	@echo "Building GUI projects..."
	$(BUILD_SCRIPT) gui $(OPTS)

.PHONY: flash
flash:
	@echo "Flashing to device..."
	$(BUILD_SCRIPT) flash $(OPTS)

# ============================================================================
# Clean Targets
# ============================================================================

.PHONY: clean
clean:
	@echo "Cleaning build artifacts..."
	$(BUILD_SCRIPT) clean

.PHONY: clean-all
clean-all: clean

# ============================================================================
# Test Targets
# ============================================================================

.PHONY: test
test:
	@echo "Running tests..."
	@# Add test commands here

.PHONY: check
check: test

# ============================================================================
# Utility Targets
# ============================================================================

.PHONY: info
info:
	@echo "Project Information:"
	@echo "  Project: ESP8266 GUIKit + WebDAV Server"
	@echo "  Build Script: $(BUILD_SCRIPT)"
	@echo "  SDK Path: $(SDK_PATH)"
	@echo ""
	@./build.sh version

.PHONY: env
env:
	@echo "Environment Variables:"
	@echo "  BUILD_SCRIPT=$(BUILD_SCRIPT)"
	@echo "  SDK_PATH=$(SDK_PATH)"
	@echo "  BOOTLOADER_ENV=$(BOOTLOADER_ENV)"
	@echo "  KERNEL_ENV=$(KERNEL_ENV)"
	@echo ""
	@echo "Optional Variables:"
	@echo "  VERBOSE=$(VERBOSE)"
	@echo "  UPLOAD=$(UPLOAD)"
	@echo "  PORT=$(PORT)"
	@echo "  SDCARD=$(SDCARD)"

# ============================================================================
# Option Handling
# ============================================================================

# Build options from make variables
OPTS =
ifdef VERBOSE
  ifeq ($(VERBOSE),1)
    OPTS += --debug
  endif
endif

ifdef UPLOAD
  ifeq ($(UPLOAD),1)
    OPTS += --upload
  endif
endif

ifdef PORT
  OPTS += --port $(PORT)
endif

ifdef SDCARD
  OPTS += --sd $(SDCARD)
endif

# ============================================================================
# File Targets (for dependency tracking)
# ============================================================================

# These targets help with make's dependency tracking
.PHONY: platformio.ini
platformio.ini:

# Include dependency files
-include $(wildcard src/**/*.h src/**/*.cpp src/**/*.ino)
