#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Floating_Point_Exceptions.hpp>
#include <filesystem>

namespace fs = std::filesystem;

using namespace Mlib;

int main(int argc, char **argv) {
    enable_floating_point_exceptions();

    ArgParser parser(
        "Usage: sfm_dense --intrinsic_matrix <intrinsic_matrix.m> --images <images.txt> --cameras <cameras.txt>",
        {},
        {"--intrinsic_matrix", "--images", "--cameras"});

    return 0;
}
