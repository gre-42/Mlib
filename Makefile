.PHONY: all test_debug test_release distclean

CMAKE_BUILD_TYPE ?= Release

all: debug release

build:
	./build.sh ${CMAKE_BUILD_TYPE}

test:
	./build.sh ${CMAKE_BUILD_TYPE} test

build10:
	CXX=g++-10 make build CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}

test10:
	CXX=g++-10 make test CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}

debug:
	make build CMAKE_BUILD_TYPE=Debug

release:
	make build CMAKE_BUILD_TYPE=Release

test_debug: debug
	make test CMAKE_BUILD_TYPE=Debug

test_release: release
	make test CMAKE_BUILD_TYPE=Release

debug10:
	make build10 CMAKE_BUILD_TYPE=Debug

release10:
	make build10 CMAKE_BUILD_TYPE=Release

release10_dbg:
	make build10 CMAKE_BUILD_TYPE=RelWithDebInfo

test_debug10: debug10
	make test10 CMAKE_BUILD_TYPE=Debug

test_release10: release10
	make test10 CMAKE_BUILD_TYPE=Release

build_clang:
	CC=/usr/bin/clang CXX=/usr/bin/clang++ BUILD_PREFIX=L \
		make build CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}

build_asan10:
	CXX=g++-10 CFLAGS=-fsanitize=address CXXFLAGS=-fsanitize=address LDFLAGS=-fsanitize=address BUILD_PREFIX=A \
		make build CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}

distclean:
	./build.sh Debug distclean
	./build.sh Release distclean
	./build.sh RelWithDebInfo distclean

clang-tidy:
	find Mlib -iname "*.cpp" -exec clang-tidy '{}' -checks=-clang-diagnostic-gnu-designator -- -I. ';'
