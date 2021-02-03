.PHONY: all test_debug test_release distclean

all: debug release

debug:
	./build.sh Debug

release:
	./build.sh Release

test_debug: debug
	./build.sh Debug test

test_release: release
	./build.sh Release test

debug10:
	CXX=g++-10 ./build.sh Debug

release10:
	CXX=g++-10 ./build.sh Release

release10_dbg:
	CXX=g++-10 ./build.sh RelWithDebInfo

test_debug10: debug10
	CXX=g++-10 ./build.sh Debug test

test_release10: release10
	CXX=g++-10 ./build.sh Release test

debug_clang:
	CC=/usr/bin/clang CXX=/usr/bin/clang++ BUILD_PREFIX=L ./build.sh Debug

distclean:
	./build.sh Debug distclean
	./build.sh Release distclean

clang-tidy:
	find Mlib -iname "*.cpp" -exec clang-tidy '{}' -checks=-clang-diagnostic-gnu-designator -- -I. ';'
