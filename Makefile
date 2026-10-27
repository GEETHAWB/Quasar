PROJECT_DIR := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))

CXX := g++
CMAKE := cmake 
BUILD_ROOT := $(PROJECT_DIR)/build
TARGET := Quasar

MODE ?= debug

ifeq ($(MODE),release)
	BUILD_DIR := $(BUILD_ROOT)/release
	BUILD_TYPE := Release
	DEBUG_MODE := OFF
else ifeq ($(MODE),debug)
	BUILD_DIR := $(BUILD_ROOT)/debug
	BUILD_TYPE := Debug
	DEBUG_MODE := ON 
else
	$(error Invalid MODE '$(MODE)'. Use release or debug)
endif

.PHONY: all build run debug clean

all: build

# Build Release
build:
	$(MAKE) --no-print-directory _build MODE=release

# Build Debug
debug:
	$(MAKE) --no-print-directory _build MODE=debug

run:
	$(MAKE) --no-print-directory _run MODE=$(MODE)

# Shortcuts
run-r:
	$(MAKE) --no-print-directory _run MODE=release

run-d:
	$(MAKE) --no-print-directory _run MODE=debug

# Internal targets

_build:
	$(CMAKE) -S "$(PROJECT_DIR)" -B $(BUILD_DIR) \
		-G Ninja \
		-DCMAKE_CXX_COMPILER=$(CXX) \
		-DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
		-DDEBUG_MODE=$(DEBUG_MODE)
	$(CMAKE) --build $(BUILD_DIR) --parallel

_run: _build
	clear
	"$(BUILD_DIR)/$(TARGET)"

clean:
	$(CMAKE) -E rm -rf $(BUILD_ROOT)
