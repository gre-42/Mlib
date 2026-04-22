.PHONY: all recastnavigation cmake build
SHELL := /bin/bash

ostype != uname

CMAKE_BUILD_TYPE ?= Release
MAKE_TARGET ?= build

all: recastnavigation cmake build

ENV :=
ifneq ($(filter MSYS% CYGWIN% MINGW%,$(ostype)),)
    PLATFORM_CHAR := M
else
    PLATFORM_CHAR := U
endif
BUILD_DIR := $(PLATFORM_CHAR)$(CMAKE_BUILD_TYPE)
ENV       := 
# ASAN
ifeq ($(ASAN),1)
    ENV       += CFLAGS=-fsanitize=address
    ENV       += CXXFLAGS=-fsanitize=address
    ENV       += LDFLAGS=-fsanitize=address
    BUILD_DIR := A$(BUILD_DIR)
endif
# TSAN
ifeq ($(TSAN),1)
    ENV       += CFLAGS=-fsanitize=thread
    ENV       += CXXFLAGS=-fsanitize=thread
    ENV       += LDFLAGS=-fsanitize=thread
    BUILD_DIR := T$(BUILD_DIR)
endif
# UBSAN
ifeq ($(UBSAN),1)
    ENV       += CFLAGS=-fsanitize=undefined
    ENV       += CXXFLAGS=-fsanitize=undefined
    ENV       += LDFLAGS=-fsanitize=undefined
    BUILD_DIR := B$(BUILD_DIR)
endif
# CLANG
ifeq ($(CLANG),1)
    ENV       += CC=clang-20 CXX=clang++-20
    BUILD_DIR := L$(BUILD_DIR)
endif
# LIBCPP
ifeq ($(LIBCPP),1)
    ENV       += CXXFLAGS=-stdlib=libc++
    BUILD_DIR := C$(BUILD_DIR)
endif
# EMSDK
PODMAN_FLAGS := $(addprefix -e ,$(ENV))
ifeq ($(EMSDK),1)
    CMAKE_CMD := podman run --rm -v "$(PWD)/Mlib:/src:Z" $(PODMAN_FLAGS) mgame/emsdk emcmake
    BUILD_CMD := podman run --rm -v "$(PWD)/Mlib:/src:Z" mgame/emsdk
else
    CMAKE_CMD :=
    BUILD_CMD :=
endif
ifeq ($(EMSDK),1)
    BUILD_DIR := E$(BUILD_DIR)
endif

echo_platform_char:
	@echo "$(PLATFORM_CHAR)"

echo_build_dir:
	@echo "$(BUILD_DIR)"

emsdk_image:
	podman build -f Dockerfile.emsdk -t mgame/emsdk

cmake:
	$(CMAKE_CMD) cmake -G Ninja -DCMAKE_BUILD_TYPE="$(CMAKE_BUILD_TYPE)" -B "$(BUILD_DIR)"

build:
	$(BUILD_CMD) cmake --build "$(BUILD_DIR)" --verbose

clang-tidy:
	find Mlib -iname "*.cpp" -exec clang-tidy '{}' -checks=-clang-diagnostic-gnu-designator -- -I. ';'

cppcheck:
	cppcheck . -i Mlib/Geometry/Primitives/Triangle_Triangle_Intersection.cpp

recastnavigation:
	mkdir -p RecastBuild
	cd RecastBuild                                  \
		&& cmake -G Ninja ../recastnavigation       \
			-DRECASTNAVIGATION_DEMO=OFF             \
			-DRECASTNAVIGATION_TESTS=OFF            \
			-DRECASTNAVIGATION_EXAMPLES=OFF         \
			-DBUILD_SHARED_LIBS=ON                  \
			-DCMAKE_BUILD_TYPE=$(CMAKE_BUILD_TYPE)  \
		&& cmake --build . --verbose
