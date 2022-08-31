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
	CC=/usr/bin/clang CXX=/usr/bin/clang++ CXXFLAGS="-Wno-unknown-warning-option -Wno-ignored-optimization-argument" BUILD_PREFIX=L${BUILD_PREFIX} \
		make build CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}

build_asan:
	CFLAGS=-fsanitize=address CXXFLAGS=-fsanitize=address LDFLAGS=-fsanitize=address BUILD_PREFIX=A \
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
		&& cmake ../recastnavigation                \
			-DRECASTNAVIGATION_DEMO=OFF             \
			-DRECASTNAVIGATION_TESTS=OFF            \
			-DRECASTNAVIGATION_EXAMPLES=OFF         \
			-DBUILD_SHARED_LIBS=ON                  \
			-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}  \
		&& cmake --build . --verbose
