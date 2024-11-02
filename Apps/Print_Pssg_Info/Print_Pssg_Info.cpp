#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Geometry/Mesh/Load/Load_Pssg.hpp>
#include <Mlib/Geometry/Mesh/Load/Pssg_Elements.hpp>
#include <Mlib/Io/Binary.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <filesystem>

using namespace Mlib;

int main(int argc, char **argv) {
    const ArgParser parser(
        "Usage: print_pssg_info <file.pssg> [file2.pssg ...] [--export <pattern>]",
        {},
        {"--export"});
    try {
        const auto args = parser.parsed(argc, argv);
        args.assert_num_unnamed_atleast(1);
        for (const auto& file : args.unnamed_values()) {
            linfo() << "Processing file " << file;
            auto pssg = load_pssg(file, IoVerbosity::METADATA);
            if (args.has_named_value("--export")) {
                DECLARE_REGEX(re, args.named_value("--export"));
                for (const auto& [name, data] : pssg.textures) {
                    if (!Mlib::re::regex_search(*name, re)) {
                        continue;
                    }
                    auto tex_filename = std::filesystem::path{ "textures" } / *name;
                    auto f = create_ofstream(tex_filename, std::ios::binary);
                    if (f->fail()) {
                        THROW_OR_ABORT("Could not open file for write: \"" + tex_filename.string() + '"');
                    }
                    f->write((const char*)data.data.data(), integral_cast<std::streamsize>(data.data.size()));
                    f->flush();
                    if (f->fail()) {
                        THROW_OR_ABORT("Could not write to file: \"" + tex_filename.string() + '"');
                    }
                }
            }
        }
    } catch (const std::exception& e) {
        lerr() << e.what();
        return 1;
    }
    return 0;
}
