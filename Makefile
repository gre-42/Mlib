.PHONY: all test_debug test_release distclean

all: debug release

debug:
	CXX=g++-10 ./build.sh Debug

release:
	CXX=g++-10 ./build.sh Release

test_debug: debug
	CXX=g++-10 ./build.sh Debug test

test_release: release
	CXX=g++-10 ./build.sh Release test

debug_clang:
	CC=/usr/bin/clang CXX=/usr/bin/clang++ BUILD_PREFIX=L ./build.sh Debug

distclean:
	./build.sh Debug distclean
	./build.sh Release distclean

clang-tidy:
	find Mlib -iname "*.cpp" -exec clang-tidy '{}' -checks=-clang-diagnostic-gnu-designator -- -I. ';'
