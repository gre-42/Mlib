#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Geometry/Mesh/Load/Img_Reader.hpp>
#include <Mlib/Geometry/Mesh/Load/Load_Dff.hpp>
#include <Mlib/Geometry/Mesh/Load/Raster_Config.hpp>
#include <Mlib/Io/Folder_IStream_Dictionary.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <filesystem>

using namespace Mlib;

static void add_resource(
    const VariableAndHash<std::string>& name,
    const std::shared_ptr<IIStreamDictionary>& img)
{
    auto extension = std::filesystem::path{ *name }.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(),
        ::tolower);
    if (extension == ".dff") {
        linfo() << "dff: " << *name;
        auto istr = img->read(name, std::ios::binary, CURRENT_SOURCE_LOCATION);
        Dff::read_dff(*istr.stream, IoVerbosity::METADATA);
    } else if (extension == ".txd") {
        linfo() << "txd: " << *name;
        Dff::read_txd(
            *img->read(name, std::ios::binary, CURRENT_SOURCE_LOCATION).stream,
            nullptr,
            nullptr,
            IoVerbosity::METADATA);
    } else {
        THROW_OR_ABORT("Unknown resource type: \"" + *name + "\". Extension: \"" + extension + '"');
    }
}

static void add_file_resource(const std::string& name)
{
    auto extension = std::filesystem::path{ name }.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(),
        ::tolower);
    if (extension == ".img") {
        linfo() << "img: " << name;
        auto img = ImgReader::load_from_file(name);
        for (const auto& name : img->names()) {
            add_resource(name, img);
        }
    } else {
        linfo() << "path: " << name;
        auto path = std::filesystem::path{ name };
        auto dir = std::make_shared<FolderIStreamDictionary>(path.parent_path().string());
        add_resource(VariableAndHash<std::string>{path.filename().string()}, dir);
    }
}

int main(int argc, char **argv) {
    const ArgParser parser(
        "Usage: print_dff_info <file.{dff,img}> [file2.{dff,img} ...]",
        {},
        {});
    try {
        const auto args = parser.parsed(argc, argv);
        args.assert_num_unnamed_atleast(1);
        for (const auto& file : args.unnamed_values()) {
            add_file_resource(file);
        }
    } catch (const std::exception& e) {
        lerr() << e.what();
        return 1;
    }
    return 0;
}
