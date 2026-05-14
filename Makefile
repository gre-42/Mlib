.PHONY: all recastnavigation cmake build
SHELL := /bin/bash

ostype != uname

CMAKE_BUILD_TYPE ?= Release
MAKE_TARGET ?= build

all: recastnavigation cmake build

ENV           :=
CMAKE_CMD     :=
CONTAINER     := unknown_container
CMAKE_OPTIONS :=
ifneq ($(filter MSYS% CYGWIN% MINGW%,$(ostype)),)
    PLATFORM_CHAR := M
else
    PLATFORM_CHAR := U
endif
BUILD_DIR := $(PLATFORM_CHAR)$(CMAKE_BUILD_TYPE)
DEPEND_PREFIX := $(PLATFORM_CHAR)
# ASAN
ifeq ($(ASAN),1)
    CFLAGS    += -fsanitize=address
    CXXFLAGS  += -fsanitize=address
    LDFLAGS   += -fsanitize=address
    BUILD_DIR := A$(BUILD_DIR)
endif
# TSAN
ifeq ($(TSAN),1)
    CFLAGS    += -fsanitize=thread
    CXXFLAGS  += -fsanitize=thread
    LDFLAGS   += -fsanitize=thread
    BUILD_DIR := T$(BUILD_DIR)
endif
# UBSAN
ifeq ($(UBSAN),1)
    CFLAGS    += -fsanitize=undefined
    CXXFLAGS  += -fsanitize=undefined
    LDFLAGS   += -fsanitize=undefined
    BUILD_DIR := B$(BUILD_DIR)
endif
ifeq ($(HEADLESS),1)
    CMAKE_OPTIONS := -D BUILD_AUDIO=OFF -D BUILD_GRAPHICS=OFF -D BUILD_ICU=OFF
    BUILD_DIR     := H$(BUILD_DIR)
endif
# CLANG
ifeq ($(CLANG),1)
    # If the default Clang version is too old, pick Clang 20.
    DEFAULT_CLANG_VERSION := $(shell clang --version | sed -nE 's/.*version ([0-9]+).*/\1/p')
    CLANG_SUFFIX := $(shell if [[ "$(DEFAULT_CLANG_VERSION)" -lt 20 ]]; then echo 20; else echo ""; fi)
    ENV       += CC=clang$(CLANG_SUFFIX) CXX=clang++$(CLANG_SUFFIX)
    BUILD_DIR := L$(BUILD_DIR)
endif
# LIBCPP
ifeq ($(LIBCPP),1)
    CXXFLAGS  += -stdlib=libc++
    BUILD_DIR := C$(BUILD_DIR)
endif
# EMSDK, EMSDK32
ES_LDFLAGS_COMMON := -pthread -sWASM_BIGINT -sSTACK_SIZE=1MB -sPTHREAD_POOL_SIZE=8 -sMALLOC=mimalloc -fexceptions
ifneq (,$(filter $(CMAKE_BUILD_TYPE),Debug RelWithDebInfo))
    ES_LDFLAGS_COMMON := ${ES_LDFLAGS_COMMON} -sASSERTIONS=2
else
    ES_LDFLAGS_COMMON := ${ES_LDFLAGS_COMMON} -sASSERTIONS=0
endif
ifeq ($(EMSDK),1)
    CFLAGS    += -sMEMORY64=1 -pthread
    CXXFLAGS  += -sMEMORY64=1 -pthread -fexceptions
    LDFLAGS   += ${ES_LDFLAGS_COMMON} -sMEMORY64=1 -sINITIAL_MEMORY=4294967296 -sALLOW_MEMORY_GROWTH=0
    BUILD_DIR     := E$(BUILD_DIR)
    DEPEND_PREFIX := E
    CMAKE_CMD     := emcmake
    CONTAINER     := mgame/emsdk
endif
ifeq ($(EMSDK32),1)
    CFLAGS    += -sMEMORY64=0 -pthread
    CXXFLAGS  += -sMEMORY64=0 -pthread -fexceptions
    LDFLAGS   += ${ES_LDFLAGS_COMMON} -sMEMORY64=0 -sINITIAL_MEMORY=2146435072 -sALLOW_MEMORY_GROWTH=0
    BUILD_DIR     := E32$(BUILD_DIR)
    DEPEND_PREFIX := E32
    CMAKE_CMD     := emcmake
    CONTAINER     := mgame/emsdk
endif
ifeq ($(PROF),1)
    CFLAGS    += --profiling
    CXXFLAGS  += --profiling
    LDFLAGS   += --profiling
    BUILD_DIR     := P$(BUILD_DIR)
    DEPEND_PREFIX := P$(DEPEND_PREFIX)
endif
ENV += CFLAGS="$(CFLAGS)"
ENV += CXXFLAGS="$(CXXFLAGS)"
ENV += LDFLAGS="$(LDFLAGS)"
ifeq ($(PODMAN),1)
    PODMAN_FLAGS := $(addprefix -e ,$(ENV))
    WDIR      := $(shell realpath -s --relative-to="$(PWD)" "$(CURDIR)")
    CMAKE_CMD := podman run --rm -it -v "$(PWD):/src:Z" -w "/src/$(WDIR)" $(PODMAN_FLAGS) $(CONTAINER) $(CMAKE_CMD)
    BUILD_CMD := podman run --rm -it -v "$(PWD):/src:Z" -w "/src/$(WDIR)" $(CONTAINER)
    INTER_CMD := podman run --rm -it -v "$(PWD):/src:Z" -w "/src/$(WDIR)" $(CONTAINER)
    DAEMON_CMD := podman run -d -v "$(PWD):/src:Z" -w "/src/$(WDIR)" $(CONTAINER)
else
    CMAKE_CMD := $(ENV) $(CMAKE_CMD)
    BUILD_CMD :=
    INTER_CMD := $(ENV)
    DAEMON_CMD := DAEMON_CMD
endif

echo_platform_char:
	@echo "$(PLATFORM_CHAR)"

echo_build_dir:
	@echo "$(BUILD_DIR)"

emsdk_image:
	podman build -f Dockerfile.emsdk -t mgame/emsdk

cmake:
	$(CMAKE_CMD) cmake -G Ninja -DCMAKE_BUILD_TYPE="$(CMAKE_BUILD_TYPE)" -B "$(BUILD_DIR)" -D DEPEND_PREFIX=$(DEPEND_PREFIX) $(CMAKE_OPTIONS)

cmake_fresh:
	# This is the same as "cmake $(BUILD_DIR) --fresh"
	rm -rf "$(BUILD_DIR)/CMakeCache.txt" "$(BUILD_DIR)/CMakeFiles"
	rm -rf "$(DEPEND_PREFIX)RecastBuild/CMakeCache.txt" "$(DEPEND_PREFIX)RecastBuild/CMakeFiles"

build:
	$(BUILD_CMD) cmake --build "$(BUILD_DIR)" --verbose

login:
	$(INTER_CMD) bash

daemon:
	$(DAEMON_CMD) sleep infinity

empackage:
	$(INTER_CMD) /emsdk/upstream/emscripten/tools/file_packager /src/static/assets.data --preload /src/static/data@/ --js-output=/src/static/assets.js

clang-tidy:
	find Mlib -iname "*.cpp" -exec clang-tidy '{}' -checks=-clang-diagnostic-gnu-designator -- -I. ';'

cppcheck:
	cppcheck . -i Mlib/Geometry/Primitives/Triangle_Triangle_Intersection.cpp

recastnavigation:
	mkdir -p $(DEPEND_PREFIX)RecastBuild
	$(CMAKE_CMD) cmake                              \
		-B $(DEPEND_PREFIX)RecastBuild              \
		-S recastnavigation                         \
		-G Ninja                                    \
		-DRECASTNAVIGATION_DEMO=OFF                 \
		-DRECASTNAVIGATION_TESTS=OFF                \
		-DRECASTNAVIGATION_EXAMPLES=OFF             \
		-DBUILD_SHARED_LIBS=ON                      \
		-DCMAKE_BUILD_TYPE=$(CMAKE_BUILD_TYPE)
	$(BUILD_CMD) cmake --build $(DEPEND_PREFIX)RecastBuild --verbose
