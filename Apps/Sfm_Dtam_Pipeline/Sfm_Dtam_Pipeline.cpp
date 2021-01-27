#include <Mlib/Arg_Parser.hpp>
#include <fenv.h>
#include <filesystem>

namespace fs = std::filesystem;

#ifdef _MSC_VER
#pragma float_control(except, on)
#endif

int main(int argc, char **argv) {
    #ifdef __linux__
    feenableexcept(FE_INVALID);
    #endif

    ArgParser parser(
        "Usage: sfm_dense --intrinsic_matrix <intrinsic_matrix.m> --images <images.txt> --cameras <cameras.txt>",
        {},
        {"--intrinsic_matrix", "--images", "--cameras"});

    return 0;
}
