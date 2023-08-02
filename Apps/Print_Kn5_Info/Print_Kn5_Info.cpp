#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Geometry/Mesh/Load/Load_Kn5.hpp>

using namespace Mlib;

int main(int argc, char **argv) {
    const ArgParser parser(
        "Usage: print_kn5_info <file.kn5>",
        {},
        {});
    const auto args = parser.parsed(argc, argv);
    args.assert_num_unnamed(1);
    load_kn5(args.unnamed_value(0), true);
    return 0;
}
