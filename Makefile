.PHONY: all test_debug test_release distclean recastnavigation

CMAKE_BUILD_TYPE ?= Release

all: recastnavigation build

build:
	./build.sh ${CMAKE_BUILD_TYPE}

test:
	./build.sh ${CMAKE_BUILD_TYPE} test

debug:
	make build CMAKE_BUILD_TYPE=Debug

release:
	make build CMAKE_BUILD_TYPE=Release

test_debug: debug
	make test CMAKE_BUILD_TYPE=Debug

test_release: release
	make test CMAKE_BUILD_TYPE=Release

release_dbg:
	make build CMAKE_BUILD_TYPE=RelWithDebInfo

build_clang:
	CC=clang CXX=clang++ BUILD_PREFIX=L${BUILD_PREFIX} \
		make build CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}

build_clang_libcpp:
	CXXFLAGS=-stdlib=libc++ CC=clang CXX=clang++ BUILD_PREFIX=C${BUILD_PREFIX} \
		 make build CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}

build_asan:
	CFLAGS=-fsanitize=address CXXFLAGS=-fsanitize=address LDFLAGS=-fsanitize=address BUILD_PREFIX=A${BUILD_PREFIX} \
		make build CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}

build_tsan:
	CFLAGS=-fsanitize=thread CXXFLAGS=-fsanitize=thread LDFLAGS=-fsanitize=thread BUILD_PREFIX=T${BUILD_PREFIX} \
		make build CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}

build_ubsan:
	CFLAGS=-fsanitize=undefined CXXFLAGS=-fsanitize=undefined LDFLAGS=-fsanitize=undefined BUILD_PREFIX=B${BUILD_PREFIX} \
                make build CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}

build_asan_clang:
	CC=clang CXX=clang++ \
	CFLAGS=-fsanitize=address CXXFLAGS=-fsanitize=address LDFLAGS=-fsanitize=address BUILD_PREFIX=LA${BUILD_PREFIX} \
		make build CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}

build_tsan_clang:
	CC=clang CXX=clang++ \
	CFLAGS=-fsanitize=thread CXXFLAGS=-fsanitize=thread LDFLAGS=-fsanitize=thread BUILD_PREFIX=LT${BUILD_PREFIX} \
		make build CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}

distclean:
	./build.sh Debug distclean
	./build.sh Release distclean
	./build.sh RelWithDebInfo distclean

clang-tidy:
	find Mlib -iname "*.cpp" -exec clang-tidy '{}' -checks=-clang-diagnostic-gnu-designator -- -I. ';'

cppcheck:
	cppcheck . -i Mlib/Geometry/Intersection/Triangle_Triangle_Intersection.cpp

recastnavigation:
	mkdir -p RecastBuild
	cd RecastBuild                                  \
		&& cmake -G Ninja ../recastnavigation       \
			-DRECASTNAVIGATION_DEMO=OFF             \
			-DRECASTNAVIGATION_TESTS=OFF            \
			-DRECASTNAVIGATION_EXAMPLES=OFF         \
			-DBUILD_SHARED_LIBS=ON                  \
			-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}  \
		&& cmake --build . --verbose
