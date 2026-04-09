.PHONY: all recastnavigation cmake build

CMAKE_BUILD_TYPE ?= Release
MAKE_TARGET ?= build

all: recastnavigation cmake build

ENV =
BUILD_DIR = U$(CMAKE_BUILD_TYPE)
# ASAN
ENV !=                                            \
    if [ "$(ASAN)" = 1 ]; then                    \
        echo "$(ENV)                              \
            CFLAGS=-fsanitize=address             \
            CXXFLAGS=-fsanitize=address           \
            LDFLAGS=-fsanitize=address" |         \
            sed "s/ +/ /g";                       \
    else                                          \
        echo "$(ENV)";                            \
    fi
BUILD_DIR !=                                      \
    if [ "$(ASAN)" = 1 ]; then                    \
        echo "A$(BUILD_DIR)";                     \
    else                                          \
        echo "$(BUILD_DIR)";                      \
    fi
# TSAN
ENV !=                                            \
    if [ "$(TSAN)" = 1 ]; then                    \
        echo "$(ENV)                              \
            CFLAGS=-fsanitize=thread              \
            CXXFLAGS=-fsanitize=thread            \
            LDFLAGS=-fsanitize=thread" |          \
            sed "s/ +/ /g";                       \
    else                                          \
        echo "$(ENV)";                            \
    fi
BUILD_DIR !=                                      \
    if [ "$(TSAN)" = 1 ]; then                    \
        echo "T$(BUILD_DIR)";                     \
    else                                          \
        echo "$(BUILD_DIR)";                      \
    fi
# UBSAN
ENV !=                                            \
    if [ "$(UBSAN)" = 1 ]; then                   \
        echo "$(ENV)                              \
            CFLAGS=-fsanitize=undefined           \
            CXXFLAGS=-fsanitize=undefined         \
            LDFLAGS=-fsanitize=undefined" |       \
            sed "s/ +/ /g";                       \
    else                                          \
        echo "$(ENV)";                            \
    fi
BUILD_DIR !=                                      \
    if [ "$(UBSAN)" = 1 ]; then                   \
        echo "B$(BUILD_DIR)";                     \
    else                                          \
        echo "$(BUILD_DIR)";                      \
    fi                                            \
# CLANG
ENV !=                                            \
    if [ "$(CLANG)" = 1 ]; then                   \
        echo "$(ENV) CC=clang-20 CXX=clang++-20"; \
    fi
BUILD_DIR !=                                      \
    if [ "$(CLANG)" = 1 ]; then                   \
        echo "L$(BUILD_DIR)";                     \
    else                                          \
        echo "$(BUILD_DIR)";                      \
    fi
# LIBCPP
ENV !=                                            \
    if [ "$(LIBCPP)" = 1 ]; then                  \
        echo "$(ENV) CXXFLAGS=-stdlib=libc++";    \
    fi
BUILD_DIR !=                                      \
    if [ "$(LIBCPP)" = 1 ]; then                  \
        echo "C$(BUILD_DIR)";                     \
    else                                          \
        echo "$(BUILD_DIR)";                      \
    fi

echo_build_dir:
	@echo "$(BUILD_DIR)"

cmake:
	$(ENV) cmake -G Ninja -DCMAKE_BUILD_TYPE="$(CMAKE_BUILD_TYPE)" -B "$(BUILD_DIR)"

build:
	cmake --build "$(BUILD_DIR)" --verbose

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
