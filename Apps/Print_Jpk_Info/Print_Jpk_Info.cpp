#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Geometry/Mesh/Load/Jpk_Reader.hpp>
#include <Mlib/Io/Binary.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <filesystem>

using namespace Mlib;

int main(int argc, char **argv) {
    const ArgParser parser(
        "Usage: print_jpk_info <file.jpk> [file2.jpk ...] [--export <pattern>]",
        {},
        {"--export"});
    try {
        const auto args = parser.parsed(argc, argv);
        args.assert_num_unnamed_atleast(1);
        for (const auto& file : args.unnamed_values()) {
            linfo() << "Processing file " << file;
            auto reader = JpkReader::load_from_file(file, IoVerbosity::METADATA);
            if (args.has_named_value("--export")) {
                DECLARE_REGEX(re, args.named_value("--export"));
                for (const auto& name : reader->names()) {
                    if (!Mlib::re::regex_search(name, re)) {
                        continue;
                    }
                    auto element_filename = std::filesystem::path{ "jpk_element" } / name;
                    auto f = create_ofstream(element_filename, std::ios::binary);
                    if (f->fail()) {
                        THROW_OR_ABORT("Could not open file for write: \"" + element_filename.string() + '"');
                    }
                    auto s = reader->read(name, std::ios::binary, CURRENT_SOURCE_LOCATION);
                    for (std::streamsize i = 0; i < s.size; ++i) {
                        char c;
                        *s.stream >> c;
                        if (s.stream->fail()) {
                            THROW_OR_ABORT("Could not read from \"" + file + '"');
                        }
                        *f << c;
                        if (f->fail()) {
                            THROW_OR_ABORT("Could not write to \"" + element_filename.string() + '"');
                        }
                    }
                    f->flush();
                    if (f->fail()) {
                        THROW_OR_ABORT("Could not write to file: \"" + element_filename.string() + '"');
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
