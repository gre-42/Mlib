#include <Mlib/Geometry/Mesh/Load/Load_Kn5.hpp>
#include <Mlib/Io/Arg_Parser.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Os/Utf8_Path.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <boost/regex/icu.hpp>

using namespace Mlib;

int main(int argc, char **argv) {
    const ArgParser parser(
        "Usage: print_kn5_info <file.kn5> [file2.kn5 ...] [--export <pattern>]",
        {},
        {"--export"});
    try {
        const auto args = parser.parsed(argc, argv);
        args.assert_num_unnamed_atleast(1);
        for (const auto& file_string : args.unnamed_values()) {
            auto file = Utf8Path{file_string};
            linfo() << "Processing file " << file;
            auto kn5 = load_kn5(file, true /* verbose */);
            if (args.has_named_value("--export")) {
                auto re = boost::make_u32regex(args.named_value("--export"));
                for (const auto& [name, data] : kn5.textures) {
                    if (!boost::u32regex_search(*name, re)) {
                        continue;
                    }
                    auto tex_filename = Utf8Path{ "textures" } / *name;
                    auto f = create_ofstream(tex_filename, std::ios::binary);
                    if (f->fail()) {
                        throw std::runtime_error("Could not open file for write: \"" + tex_filename.string() + '"');
                    }
                    f->write((const char*)data.data.data(), integral_cast<std::streamsize>(data.data.size()));
                    f->flush();
                    if (f->fail()) {
                        throw std::runtime_error("Could not write to file: \"" + tex_filename.string() + '"');
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
