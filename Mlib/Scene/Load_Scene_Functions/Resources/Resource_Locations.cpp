#include "Resource_Locations.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Mesh/Load/Draw_Distance_Db.hpp>
#include <Mlib/Geometry/Mesh/Load/Img_Reader.hpp>
#include <Mlib/Geometry/Mesh/Load/Load_Dff.hpp>
#include <Mlib/Geometry/Mesh/Load/Load_Dff_Array.hpp>
#include <Mlib/Geometry/Mesh/Load/Load_Mesh_Config.hpp>
#include <Mlib/Geometry/Mesh/Load/Load_Mesh_Config_Json.hpp>
#include <Mlib/Geometry/Mesh/Load/Raster_Config.hpp>
#include <Mlib/Io/Folder_IStream_Dictionary.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Render/Deallocate/Render_Allocator.hpp>
#include <Mlib/Render/Raster/Raster_Factory.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Resource_Managers/Rendering_Resources.hpp>
#include <Mlib/Render/Resources/Dff_File_Resource.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(resource_files);
DECLARE_ARGUMENT(ide_files);
DECLARE_ARGUMENT(double_precision);
DECLARE_ARGUMENT(config);
DECLARE_ARGUMENT(resource_variable);
DECLARE_ARGUMENT(texture);
}

const std::string ResourceLocations::key = "resource_locations";

LoadSceneJsonUserFunction ResourceLocations::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    execute(args);
};

template <class TPosition>
static void add_resource(
    const std::string& name,
    const std::shared_ptr<IIStreamDictionary>& img,
    const std::shared_ptr<LoadMeshConfig<TPosition>>& cfg,
    const std::shared_ptr<DrawDistanceDb>& dddb,
    std::list<std::string>& added_scene_node_resources)
{
    auto extension = std::filesystem::path{ name }.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(),
        ::tolower);
    if (extension == ".dff") {
        added_scene_node_resources.push_back(name);
        auto& res = RenderingContextStack::primary_scene_node_resources();
        res.add_resource_loader(name, [img, cfg, name, &res, dddb]() {
            auto istr = img->read(name, std::ios::binary, CURRENT_SOURCE_LOCATION);
            return load_renderable_dff(*istr, name, *cfg, res, *dddb);
            });
    } else if (extension == ".txd") {
        auto& res = RenderingContextStack::primary_rendering_resources();
        res.set_textures_lazy([img, name, &res](){
            auto txd = Dff::read_txd(
                *img->read(name, std::ios::binary, CURRENT_SOURCE_LOCATION),
                rvalue_address(Dff::RasterFactory{}),
                rvalue_address(Dff::RasterConfig{
                    .need_to_read_back_textures = true,
                    .make_native = true,
                    .flip_gl_y_axis = true}),
                IoVerbosity::SILENT);
            for (const auto& tx : txd.textures) {
                TextureSize size{
                    .width = integral_cast<int>(tx->raster->width()),
                    .height = integral_cast<int>(tx->raster->height())
                };
                auto filename = name + '_' + *tx->name;
                std::transform(filename.begin(), filename.end(), filename.begin(),
                    ::tolower);
                auto cm = ColormapWithModifiers{
                    .filename = VariableAndHash{filename},
                    .color_mode = ColorMode::RGBA,
                    .mipmap_mode = MipmapMode::WITH_MIPMAPS
                }.compute_hash();
                if (res.contains_texture(cm)) {
                    lwarn() << "Ignoring duplicate texture \"" << *tx->name << "\" with mask \"" << *tx->mask << "\" in dictionary \"" << name << '"';
                } else {
                    res.set_texture(
                        cm,
                        tx->raster->texture_handle(),
                        &size);
                }
            }
            });
    } else {
        THROW_OR_ABORT("Unknown resource type: \"" + name + "\". Extension: \"" + extension + '"');
    }
}

template <class TPosition>
static void add_file_resource(
    const std::string& name,
    const std::shared_ptr<LoadMeshConfig<TPosition>>& cfg,
    const std::shared_ptr<DrawDistanceDb>& dddb,
    std::list<std::string>& added_scene_node_resources)
{
    auto extension = std::filesystem::path{ name }.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(),
        ::tolower);
    if (extension == ".img") {
        auto img = ImgReader::load_from_file(name);
        for (const auto& name : img->names()) {
            add_resource(name, img, cfg, dddb, added_scene_node_resources);
        }
    } else {
        auto path = std::filesystem::path{ name };
        auto dir = std::make_shared<FolderIStreamDictionary>(path.parent_path().string());
        add_resource(path.filename().string(), dir, cfg, dddb, added_scene_node_resources);
    }
}

template <class TPosition>
static void exec(
    const LoadSceneJsonUserFunctionArgs& args,
    const std::shared_ptr<DrawDistanceDb>& dddb)
{
    std::list<std::string> added_scene_node_resources;
    auto cfg = std::make_shared<LoadMeshConfig<TPosition>>();
    *cfg = load_mesh_config_from_json<TPosition>(args.arguments.child(KnownArgs::config));
    if (auto c = args.arguments.try_at<VariableAndHash<std::string>>(KnownArgs::texture)) {
        auto& rr = RenderingContextStack::primary_rendering_resources();
        cfg->textures = { rr.get_blend_map_texture(*c) };
    }
    for (const auto& s : args.arguments.pathes_or_variables(KnownArgs::resource_files)) {
        add_file_resource(s.path, cfg, dddb, added_scene_node_resources);
    }
    if (auto rv = args.arguments.try_at<std::string>(KnownArgs::resource_variable)) {
        if (args.local_json_macro_arguments == nullptr) {
            THROW_OR_ABORT("No local arguments set");
        }
        args.local_json_macro_arguments->set(*rv, added_scene_node_resources);
    }
}

void ResourceLocations::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto dddb = std::make_shared<DrawDistanceDb>();
    for (const auto& f : args.arguments.pathes_or_variables(KnownArgs::ide_files)) {
        dddb->add_ide(f.path);
    }
    if (args.arguments.at<bool>(KnownArgs::double_precision)) {
        exec<double>(args, dddb);
    } else {
        exec<float>(args, dddb);
    }
}
