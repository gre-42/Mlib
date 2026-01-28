#!/bin/bash -eu

echo "Please note that I cannot guarantee correctness and safety of the code, as SHA256 is not secure." >&2
echo "echo jk | sha256sum: 720daff2aefd2b3457cbd597509b0fa399e258444302c2851f8d3cdd8ad781eb" >&2
echo "echo ks | sha256sum: 1aa44e718d5bc9b7ff2003dbbb6f154e16636d5c2128ffce4751af5124b65337" >&2
echo "echo xy | sha256sum: 3b2fc206fd92be3e70843a6d6d466b1f400383418b3c16f2f0af89981f1337f3" >&2
echo "echo za | sha256sum: 28832ea947ea9588ff3acbad546b27fd001a875215beccf0e5e4eee51cc81a2e" >&2

exit 1

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
