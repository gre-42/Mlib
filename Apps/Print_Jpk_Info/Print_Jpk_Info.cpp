#include <Mlib/Geometry/Mesh/Load/Jpk_Reader.hpp>
#include <Mlib/Io/Arg_Parser.hpp>
#include <Mlib/Io/Binary.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Strings/Utf8_Path.hpp>
#include <boost/regex/icu.hpp>

using namespace Mlib;

int main(int argc, char **argv) {
    const ArgParser parser(
        "Usage: print_jpk_info <file.jpk> [file2.jpk ...] [--export <pattern>]",
        {},
        {"--export"});
    try {
        const auto args = parser.parsed(argc, argv);
        args.assert_num_unnamed_atleast(1);
        for (const auto& file_string : args.unnamed_values()) {
            auto file = Utf8Path(file_string);
            linfo() << "Processing file " << file;
            auto reader = JpkReader::load_from_file(file, IoVerbosity::METADATA);
            if (args.has_named_value("--export")) {
                auto re = boost::make_u32regex(args.named_value("--export"));
                for (const auto& name : reader->names()) {
                    if (!boost::u32regex_search(*name, re)) {
                        continue;
                    }
                    auto element_filename = Utf8Path{ "jpk_element" } / *name;
                    auto f = create_ofstream(element_filename, std::ios::binary);
                    if (f->fail()) {
                        throw std::runtime_error("Could not open file for write: \"" + element_filename.string() + '"');
                    }
                    auto s = reader->read(name, std::ios::binary, CURRENT_SOURCE_LOCATION);
                    for (std::streamsize i = 0; i < s.size; ++i) {
                        char c;
                        *s.stream >> c;
                        if (s.stream->fail()) {
                            throw std::runtime_error("Could not read from \"" + file.string() + '"');
                        }
                        *f << c;
                        if (f->fail()) {
                            throw std::runtime_error("Could not write to \"" + element_filename.string() + '"');
                        }
                    }
                    f->flush();
                    if (f->fail()) {
                        throw std::runtime_error("Could not write to file: \"" + element_filename.string() + '"');
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
