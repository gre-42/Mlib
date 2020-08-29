#include <Mlib/Arg_Parser.hpp>
#include <fenv.h>
#include <filesystem>

namespace fs = std::filesystem;

int main(int argc, char **argv) {
    #ifndef __MINGW32__
    feenableexcept(FE_INVALID);
    #endif

    ArgParser parser(
        "Usage: sfm_dense --intrinsic_matrix <intrinsic_matrix.m> --images <images.txt> --cameras <cameras.txt>",
        {},
        {"--intrinsic_matrix", "--images", "--cameras"});

    return 0;
}
