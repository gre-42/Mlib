#!/bin/bash -eu

cd "$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

function exit_error() { echo "Usage: $(basename "$0") {Debug|Release|RelWithDebInfo} [distclean|test] [--verbose] [source-dirs...]" >&2; exit 1; }

if [ $# -lt 1 ]; then exit_error; fi

if [[ "$1" == Debug || "$1" == Release || "$1" == RelWithDebInfo ]]; then
    CMAKE_BUILD_TYPE=$1
    shift
else
    exit_error
fi
if [[ $# -ge 1 ]] && [[ "$1" == distclean ||  "$1" == test ]]; then
    TARGET=$1
    shift
else
    TARGET=""
fi

if [[ $# -ge 1 && "$1" == --verbose ]]; then
    set -x
    shift
fi

if [ $# -ne 0 ]; then
    SOURCE_DIRS="$@"
else
    SOURCE_DIRS="."
fi

for d in $SOURCE_DIRS; do
    echo "Project: $d"
    if [[ "$OSTYPE" == "msys" || "$OSTYPE" == "cygwin" ]] ; then
        DIR=$d/${BUILD_PREFIX-}M$CMAKE_BUILD_TYPE
    else
        DIR=$d/${BUILD_PREFIX-}U$CMAKE_BUILD_TYPE
    fi
    if [[ "$TARGET" == distclean ]]; then
        (
            set -x
            rm -rf $DIR
        )
    elif [[ "$TARGET" == "test" ]]; then
        cd $DIR
        ctest -VV
        cd -
    elif [[ "$TARGET" == "" ]]; then
        mkdir -p $DIR
        cd $DIR
        cmake -G Ninja ${CMAKE_OPTIONS-} -DCMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE ../
        cmake --build . --verbose
        # ninja -v
        cd -
    else
        exit_error
    fi
done
