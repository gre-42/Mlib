#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Geometry/Mesh/Load/Load_Kn5.hpp>

using namespace Mlib;

int main(int argc, char** argv) {

    const ArgParser parser(
        "Usage: load_kn5 <filename>",
        {},
        {});
    try {
        const auto args = parser.parsed(argc, argv);

        args.assert_num_unnamed(1);

        load_kn5(args.unnamed_value(0));
    } catch (const CommandLineArgumentError& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}
