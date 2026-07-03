.PHONY: all recastnavigation cmake build
SHELL := /bin/bash

ostype != uname

CMAKE_BUILD_TYPE ?= Release
MAKE_TARGET ?= build

all: recastnavigation cmake build

ifeq ($(PODMAN),1)
    E_FLAG := -e
else
    E_FLAG :=
endif
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
    CMAKE_OPTIONS := $(CMAKE_OPTIONS) -D BUILD_AUDIO=OFF -D BUILD_GRAPHICS=OFF -D BUILD_ICU=OFF
    BUILD_DIR     := H$(BUILD_DIR)
endif
ifeq ($(LAME),1)
    CMAKE_OPTIONS := $(CMAKE_OPTIONS) -D WITH_LAME=ON
    BUILD_DIR     := A$(BUILD_DIR)
endif
# CLANG
ifeq ($(CLANG),1)
    # If the default Clang version is too old, pick Clang 20.
    DEFAULT_CLANG_VERSION := $(shell clang --version | sed -nE 's/.*version ([0-9]+).*/\1/p')
    CLANG_SUFFIX := $(shell if [[ "$(DEFAULT_CLANG_VERSION)" -lt 20 ]]; then echo 20; else echo ""; fi)
    ENV       += $(E_FLAG) CC=clang$(CLANG_SUFFIX) $(E_FLAG) CXX=clang++$(CLANG_SUFFIX)
    BUILD_DIR := L$(BUILD_DIR)
endif
# LIBCPP
ifeq ($(LIBCPP),1)
    CXXFLAGS  += -stdlib=libc++
    BUILD_DIR := C$(BUILD_DIR)
endif
# EMSDK64, EMSDK32
EM_LDFLAGS_COMMON := -pthread --bind -sMODULARIZE=1 -sEXPORT_ES6=1 -sASYNCIFY=0 -sWASM_BIGINT -sSTACK_SIZE=1MB -sPTHREAD_POOL_SIZE=8 -sMALLOC=mimalloc -fwasm-exceptions
EM_COMPILEFLAGS_COMMON := -pthread
ifneq (,$(filter $(CMAKE_BUILD_TYPE),Debug RelWithDebInfo))
    EM_LDFLAGS_COMMON := ${EM_LDFLAGS_COMMON} -sASSERTIONS=2
else
    EM_LDFLAGS_COMMON := ${EM_LDFLAGS_COMMON} -sASSERTIONS=0
endif
SHARED := ON
ifeq ($(EMSDK64),1)
    CFLAGS    += -sMEMORY64=1 ${EM_COMPILEFLAGS_COMMON}
    CXXFLAGS  += -sMEMORY64=1 ${EM_COMPILEFLAGS_COMMON} -fwasm-exceptions
    LDFLAGS   += ${EM_LDFLAGS_COMMON} -sMEMORY64=1 -sINITIAL_MEMORY=4294967296 -sALLOW_MEMORY_GROWTH=0
    BUILD_DIR     := E64$(BUILD_DIR)
    DEPEND_PREFIX := E64
    CMAKE_CMD     := /emsdk/upstream/emscripten/emcmake
    CONTAINER     := mgame/emsdk
    SHARED        := OFF
endif
ifeq ($(EMSDK32),1)
    CFLAGS    += -sMEMORY64=0 ${EM_COMPILEFLAGS_COMMON}
    CXXFLAGS  += -sMEMORY64=0 ${EM_COMPILEFLAGS_COMMON} -fwasm-exceptions
    LDFLAGS   += ${EM_LDFLAGS_COMMON} -sMEMORY64=0 -sINITIAL_MEMORY=2146435072 -sALLOW_MEMORY_GROWTH=0
    BUILD_DIR     := E32$(BUILD_DIR)
    DEPEND_PREFIX := E32
    CMAKE_CMD     := /emsdk/upstream/emscripten/emcmake
    CONTAINER     := mgame/emsdk
    SHARED        := OFF
endif
ifeq ($(PROF),1)
    CFLAGS    += --profiling
    CXXFLAGS  += --profiling
    LDFLAGS   += --profiling
    BUILD_DIR     := P$(BUILD_DIR)
    DEPEND_PREFIX := P$(DEPEND_PREFIX)
endif
ENV += $(E_FLAG) CFLAGS="$(CFLAGS)"
ENV += $(E_FLAG) CXXFLAGS="$(CXXFLAGS)"
ENV += $(E_FLAG) LDFLAGS="$(LDFLAGS)"
ifeq ($(PODMAN),1)
    WDIR      := $(shell realpath -s --relative-to="$(PWD)" "$(CURDIR)")
    CMAKE_CMD := podman run --rm -it -v "$(PWD):/src:Z" -v emscripten_cache:/emsdk/upstream/emscripten/cache -w "/src/$(WDIR)" $(ENV) $(CONTAINER) $(CMAKE_CMD)
    BUILD_CMD := podman run --rm -it -v "$(PWD):/src:Z" -v emscripten_cache:/emsdk/upstream/emscripten/cache -w "/src/$(WDIR)" $(CONTAINER)
    INTER_CMD := podman run --rm -it -v "$(PWD):/src:Z" -v emscripten_cache:/emsdk/upstream/emscripten/cache -w "/src/$(WDIR)" $(CONTAINER)
    DAEMON_CMD := podman run --rm -d -v "$(PWD):/src:Z" -v emscripten_cache:/emsdk/upstream/emscripten/cache -w "/src/$(WDIR)" -p 2222:22 $(CONTAINER)
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
	podman build -f Dockerfile.build.emsdk -t mgame/emsdk

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
		-DBUILD_SHARED_LIBS=$(SHARED)               \
		-DCMAKE_BUILD_TYPE=$(CMAKE_BUILD_TYPE)
	$(BUILD_CMD) cmake --build $(DEPEND_PREFIX)RecastBuild --verbose
