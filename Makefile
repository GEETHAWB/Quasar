PROJECT_DIR := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))

CXX := g++
CMAKE := cmake
BUILD_ROOT := $(PROJECT_DIR)/build

# Executables
TARGET := Quasar
SHADERC_TARGET := qsr-shaderc

# Slang SDK
SLANG_ROOT ?= $(HOME)/.local/opt/slang

SHADERC_SOURCE ?= $(PROJECT_DIR)/shader
SHADERC_OUTPUT ?= $(PROJECT_DIR)/shader/spirv
SHADERC_PROFILE ?= spirv_1_5
SHADERC_EXTRA_ARGS ?=

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

.PHONY: all build run run-r clean \
	shaderc shaderc-r run-shaderc run-shaderc-r \
	_configure

all: build

# Build Release
build:
	$(MAKE) --no-print-directory _build MODE=release

run:
	$(MAKE) --no-print-directory _run MODE=$(MODE)

# Shortcuts
run-r:
	$(MAKE) --no-print-directory _run MODE=release

shaderc:
	$(MAKE) --no-print-directory _run-shaderc MODE=$(MODE)


# Internal targets

_configure:
	$(CMAKE) -S "$(PROJECT_DIR)" -B "$(BUILD_DIR)" \
		-G Ninja \
		-DCMAKE_CXX_COMPILER=$(CXX) \
		-DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
		-DCMAKE_PREFIX_PATH="$(SLANG_ROOT)" \
		-DSLANG_ROOT="$(SLANG_ROOT)" \
		-DDEBUG_MODE=$(DEBUG_MODE)

_build: _configure
	$(CMAKE) --build "$(BUILD_DIR)" --parallel

_build-shaderc: _configure
	$(CMAKE) --build "$(BUILD_DIR)" \
		--target $(SHADERC_TARGET) \
		--parallel

_run-shaderc: _build-shaderc
	"$(BUILD_DIR)/bin/$(SHADERC_TARGET)" \
		--source "$(SHADERC_SOURCE)" \
		--output "$(SHADERC_OUTPUT)" \
		--profile "$(SHADERC_PROFILE)" \
		$(SHADERC_EXTRA_ARGS)

_run: _build
	"$(BUILD_DIR)/$(TARGET)"

clean:
	$(CMAKE) -E rm -rf $(BUILD_ROOT)
