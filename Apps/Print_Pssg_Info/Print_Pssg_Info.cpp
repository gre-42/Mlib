#include <Mlib/Geometry/Material/Billboard_Atlas_Instance.hpp>
#include <Mlib/Geometry/Mesh/Load/Load_Mesh_Config.hpp>
#include <Mlib/Geometry/Mesh/Load/Load_Pssg.hpp>
#include <Mlib/Geometry/Mesh/Load/Load_Pssg_Arrays.hpp>
#include <Mlib/Geometry/Mesh/Load/Pssg_Elements.hpp>
#include <Mlib/Images/Dds_Info.hpp>
#include <Mlib/Io/Arg_Parser.hpp>
#include <Mlib/Os/Io/Binary.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Strings/U32_Regex.hpp>
#include <Mlib/Strings/Utf8_Path.hpp>

using namespace Mlib;

int main(int argc, char **argv) {
    const ArgParser parser(
        "Usage: print_pssg_info <file.pssg> [file2.pssg ...] [--export <pattern>] [--to_array]",
        {"--to_array"},
        {"--export"});
    try {
        const auto args = parser.parsed(argc, argv);
        args.assert_num_unnamed_atleast(1);
        for (const auto& file : args.unnamed_values()) {
            linfo() << "Processing file " << file;
            auto model = load_pssg(file, IoVerbosity::METADATA);
            if (args.has_named("--to_array")) {
                auto arrays = load_pssg_arrays<float, float>(
                    model,
                    LoadMeshConfig<float>{},
                    #ifndef WITHOUT_GRAPHICS
                    nullptr,    // dds_resources
                    #endif
                    "",         // resource_prefix
                    IoVerbosity::METADATA);
            }
            if (args.has_named_value("--export")) {
                auto re = Mlib::make_u32regex(args.named_value("--export"));
                model.root.for_each_node([&](const PssgNode& node) {
                    if (model.schema.nodes.get(node.type_id).name != "TEXTURE") {
                        return true;
                    }
                    auto node_id = node.get_attribute("id", model.schema).string();
                    if (node_id.length() > 1'000) {
                        throw std::runtime_error("Node ID too long");
                    }
                    if (!Mlib::u32regex_search(node_id, re)) {
                        return true;
                    }
                    auto width = node.get_attribute("width", model.schema).uint32();
                    if (width > 10'000) {
                        throw std::runtime_error("Width too large");
                    }
                    auto height = node.get_attribute("height", model.schema).uint32();
                    if (height > 10'000) {
                        throw std::runtime_error("Height too large");
                    }
                    std::replace_if(node_id.begin(), node_id.end(), [](auto c) {
                        if ((c >= '0') && (c <= '9')) {
                            return false;
                        }
                        if ((c >= 'A') && (c <= 'Z')) {
                            return false;
                        }
                        if ((c >= 'a') && (c <= 'z')) {
                            return false;
                        }
                        if (c == '_') {
                            return false;
                        }
                        if (c == '.') {
                            return false;
                        }
                        return true;
                    }, '_');
                    auto data = node.texture(model.schema);
                    auto tex_filename = Utf8Path{ "textures" } / (node_id + ".dds");
                    auto f = create_ofstream(tex_filename, std::ios::binary);
                    if (f->fail()) {
                        throw std::runtime_error("Could not open file for write: \"" + tex_filename.string() + '"');
                    }
                    f->write((const char*)data.data(), integral_cast<std::streamsize>(data.size()));
                    f->flush();
                    if (f->fail()) {
                        throw std::runtime_error("Could not write to file: \"" + tex_filename.string() + '"');
                    }
                    return true;
                });
            }
        }
    } catch (const std::exception& e) {
        lerr() << e.what();
        return 1;
    }
    return 0;
}
