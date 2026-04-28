.PHONY: all recastnavigation cmake build
SHELL := /bin/bash

ostype != uname

CMAKE_BUILD_TYPE ?= Release
MAKE_TARGET ?= build

all: recastnavigation cmake build

ENV           :=
CMAKE_CMD     :=
CONTAINER     := unknown_container
ifneq ($(filter MSYS% CYGWIN% MINGW%,$(ostype)),)
    PLATFORM_CHAR := M
else
    PLATFORM_CHAR := U
endif
BUILD_DIR := $(PLATFORM_CHAR)$(CMAKE_BUILD_TYPE)
RECAST_PREFIX := $(PLATFORM_CHAR)
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
ifeq ($(EMSDK),1)
    ENV       += CFLAGS="-sMEMORY64=1 -pthread"
    ENV       += CXXFLAGS="-sMEMORY64=1 -pthread -fexceptions"
    ENV       += LDFLAGS="-sMEMORY64=1 -pthread -sWASM_BIGINT -sINITIAL_MEMORY=4294967296 -sALLOW_MEMORY_GROWTH=0 -sSTACK_SIZE=8MB -sPTHREAD_POOL_SIZE=32 -sPTHREAD_POOL_SIZE=2 -sASSERTIONS=2 -sEXIT_RUNTIME=0 -fexceptions"
    BUILD_DIR     := E$(BUILD_DIR)
    RECAST_PREFIX := E
    CMAKE_CMD     := emcmake
    CONTAINER     := mgame/emsdk
endif
ifeq ($(EMSDK32),1)
    ENV       += CFLAGS="-sMEMORY64=0 -pthread"
    ENV       += CXXFLAGS="-sMEMORY64=0 -pthread -fexceptions"
    ENV       += LDFLAGS="-sMEMORY64=0 -pthread -sWASM_BIGINT -sINITIAL_MEMORY=2147483648 -sALLOW_MEMORY_GROWTH=0 -sSTACK_SIZE=8MB -sPTHREAD_POOL_SIZE=32 -sPTHREAD_POOL_SIZE=2 -sASSERTIONS=2 -sEXIT_RUNTIME=0 -fexceptions"
    BUILD_DIR     := E32$(BUILD_DIR)
    RECAST_PREFIX := E32
    CMAKE_CMD     := emcmake
    CONTAINER     := mgame/emsdk
endif
ifeq ($(PODMAN),1)
    PODMAN_FLAGS := $(addprefix -e ,$(ENV))
    WDIR      := $(shell realpath -s --relative-to="$(PWD)" "$(CURDIR)")
    CMAKE_CMD := podman run --rm -it -v "$(PWD):/src:Z" -w "/src/$(WDIR)" $(PODMAN_FLAGS) $(CONTAINER) $(CMAKE_CMD)
    BUILD_CMD := podman run --rm -it -v "$(PWD):/src:Z" -w "/src/$(WDIR)" $(CONTAINER)
    INTER_CMD := podman run --rm -it -v "$(PWD):/src:Z" -w "/src/$(WDIR)" $(CONTAINER)
else
    CMAKE_CMD := $(ENV) $(CMAKE_CMD)
    BUILD_CMD :=
    INTER_CMD := $(ENV)
endif

echo_platform_char:
	@echo "$(PLATFORM_CHAR)"

echo_build_dir:
	@echo "$(BUILD_DIR)"

emsdk_image:
	podman build -f Dockerfile.emsdk -t mgame/emsdk

cmake:
	$(CMAKE_CMD) cmake -G Ninja -DCMAKE_BUILD_TYPE="$(CMAKE_BUILD_TYPE)" -B "$(BUILD_DIR)" -D RECAST_PREFIX=$(RECAST_PREFIX)

cmake_fresh:
	# This is the same as "cmake $(BUILD_DIR) --fresh"
	rm -rf "$(BUILD_DIR)/CMakeCache.txt" "$(BUILD_DIR)/CMakeFiles"
	rm -rf "$(RECAST_PREFIX)RecastBuild/CMakeCache.txt" "$(RECAST_PREFIX)RecastBuild/CMakeFiles"

build:
	$(BUILD_CMD) cmake --build "$(BUILD_DIR)" --verbose

login:
	$(INTER_CMD) bash

empackage:
	$(INTER_CMD) /emsdk/upstream/emscripten/tools/file_packager /src/assets.data --preload /src/data@/ --js-output=/src/assets.js

clang-tidy:
	find Mlib -iname "*.cpp" -exec clang-tidy '{}' -checks=-clang-diagnostic-gnu-designator -- -I. ';'

cppcheck:
	cppcheck . -i Mlib/Geometry/Primitives/Triangle_Triangle_Intersection.cpp

recastnavigation:
	mkdir -p $(RECAST_PREFIX)RecastBuild
	$(CMAKE_CMD) cmake                              \
		-B $(RECAST_PREFIX)RecastBuild              \
		-S recastnavigation                         \
		-G Ninja                                    \
		-DRECASTNAVIGATION_DEMO=OFF                 \
		-DRECASTNAVIGATION_TESTS=OFF                \
		-DRECASTNAVIGATION_EXAMPLES=OFF             \
		-DBUILD_SHARED_LIBS=ON                      \
		-DCMAKE_BUILD_TYPE=$(CMAKE_BUILD_TYPE)
	$(BUILD_CMD) cmake --build $(RECAST_PREFIX)RecastBuild --verbose
