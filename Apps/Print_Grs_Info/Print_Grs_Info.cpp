#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Scene_Graph/Instantiation/Read_Grs.hpp>

using namespace Mlib;

int main(int argc, char **argv) {
    const ArgParser parser(
        "Usage: print_grs_info <file.grs> [file.grs ...]",
        {},
        {});
    try {
        const auto args = parser.parsed(argc, argv);
        args.assert_num_unnamed_atleast(1);
        for (const auto& file : args.unnamed_values()) {
            linfo() << "Processing file " << file;
            Grs::load_grs(file, IoVerbosity::METADATA);
        }
    } catch (const std::exception& e) {
        lerr() << e.what();
        return 1;
    }
    return 0;
}
