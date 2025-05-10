#include <Mlib/Argument_List.hpp>
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Material/Billboard_Atlas_Instance.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array_Filter.hpp>
#include <Mlib/Geometry/Mesh/Load/Draw_Distance_Db.hpp>
#include <Mlib/Geometry/Mesh/Load/Img_Reader.hpp>
#include <Mlib/Geometry/Mesh/Load/Load_Dff.hpp>
#include <Mlib/Geometry/Mesh/Load/Load_Dff_Array.hpp>
#include <Mlib/Geometry/Mesh/Load/Load_Mesh_Config.hpp>
#include <Mlib/Geometry/Mesh/Load/Load_Pssg.hpp>
#include <Mlib/Geometry/Mesh/Load/Load_Pssg_Arrays.hpp>
#include <Mlib/Geometry/Mesh/Load/Pssg_Elements.hpp>
#include <Mlib/Geometry/Mesh/Load/Raster_Config.hpp>
#include <Mlib/Io/Folder_IStream_Dictionary.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Render/Deallocate/Render_Allocator.hpp>
#include <Mlib/Render/Raster/Raster_Factory.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Resource_Managers/Rendering_Resources.hpp>
#include <Mlib/Render/Resources/Dff_File_Resource.hpp>
#include <Mlib/Render/Resources/Pssg_File_Resource.hpp>
#include <Mlib/Scene/Json/Load_Mesh_Config_Json.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Strings/Filesystem_Path.hpp>
#include <Mlib/Threads/Thread_Top.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(filters);
DECLARE_ARGUMENT(rw_resource_files);
DECLARE_ARGUMENT(rw_ide_files);
DECLARE_ARGUMENT(pssg_files);
DECLARE_ARGUMENT(double_precision);
DECLARE_ARGUMENT(config);
DECLARE_ARGUMENT(resource_variable);
DECLARE_ARGUMENT(instantiables_variable);
DECLARE_ARGUMENT(texture);
}

template <class TPosition>
static void add_rw_resource(
    const VariableAndHash<std::string>& name,
    const std::shared_ptr<IIStreamDictionary>& img,
    const std::shared_ptr<LoadMeshConfig<TPosition>>& cfg,
    const std::shared_ptr<DrawDistanceDb>& dddb,
    std::list<VariableAndHash<std::string>>& added_scene_node_resources)
{
    auto extension = std::filesystem::path{ *name }.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(),
        ::tolower);
    if (extension == ".dff") {
        added_scene_node_resources.push_back(name);
        auto& res = RenderingContextStack::primary_scene_node_resources();
        res.add_resource_loader(name, [img, cfg, name, &res, dddb]() {
            auto istr = img->read(name, std::ios::binary, CURRENT_SOURCE_LOCATION);
            return load_renderable_dff(*istr.stream, *name, *cfg, res, *dddb);
            });
    } else if (extension == ".txd") {
        auto& res = RenderingContextStack::primary_rendering_resources();
        res.set_textures_lazy([img, name, &res](){
            auto txd = Dff::read_txd(
                *img->read(name, std::ios::binary, CURRENT_SOURCE_LOCATION).stream,
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
                auto filename = *name + '_' + *tx->name;
                std::transform(filename.begin(), filename.end(), filename.begin(),
                    ::tolower);
                auto cm = ColormapWithModifiers{
                    .filename = VariableAndHash{filename},
                    .color_mode = ColorMode::RGBA,
                    .mipmap_mode = MipmapMode::WITH_MIPMAPS
                }.compute_hash();
                if (res.contains_texture(cm)) {
                    lwarn() << "Ignoring duplicate texture \"" << *tx->name << "\" with mask \"" << *tx->mask << "\" in dictionary \"" << *name << '"';
                } else {
                    res.set_texture(
                        cm,
                        tx->raster->texture_handle(),
                        &size);
                }
            }
            });
    } else {
        THROW_OR_ABORT("Unknown resource type: \"" + *name + "\". Extension: \"" + extension + '"');
    }
}

template <class TPosition>
static void add_rw_file_resource(
    const std::string& name,
    const std::shared_ptr<LoadMeshConfig<TPosition>>& cfg,
    const std::shared_ptr<DrawDistanceDb>& dddb,
    std::list<VariableAndHash<std::string>>& added_scene_node_resources)
{
    auto extension = std::filesystem::path{ name }.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(),
        ::tolower);
    if (extension == ".img") {
        auto img = ImgReader::load_from_file(name);
        for (const auto& name : img->names()) {
            add_rw_resource(VariableAndHash<std::string>{name}, img, cfg, dddb, added_scene_node_resources);
        }
    } else {
        auto path = std::filesystem::path{ name };
        auto dir = std::make_shared<FolderIStreamDictionary>(path.parent_path().string());
        add_rw_resource(
            VariableAndHash<std::string>{path.filename().string()},
            dir,
            cfg,
            dddb,
            added_scene_node_resources);
    }
}

template <class TPosition>
static void exec(
    const LoadSceneJsonUserFunctionArgs& args,
    const std::shared_ptr<DrawDistanceDb>& dddb)
{
    std::list<VariableAndHash<std::string>> added_scene_node_resources;
    std::list<VariableAndHash<std::string>> added_instantiables;
    auto cfg = std::make_shared<LoadMeshConfig<TPosition>>();
    *cfg = load_mesh_config_from_json<TPosition>(args.arguments.child(KnownArgs::config));
    auto filters = args.arguments.at<ColoredVertexArrayFilters>(KnownArgs::filters, ColoredVertexArrayFilters{});
    if (auto c = args.arguments.try_at<VariableAndHash<std::string>>(KnownArgs::texture)) {
        auto& rr = RenderingContextStack::primary_rendering_resources();
        cfg->textures = { rr.get_blend_map_texture(*c) };
    }
    for (const auto& s : args.arguments.try_pathes_or_variables(KnownArgs::rw_resource_files)) {
        FunctionGuard fg{ "Load RW \"" + short_path(s.path) + '"' };
        add_rw_file_resource(s.path, cfg, dddb, added_scene_node_resources);
    }
    for (const auto& s : args.arguments.try_pathes_or_variables(KnownArgs::pssg_files)) {
        FunctionGuard fg{ "Load PSSG \"" + short_path(s.path) + '"' };
        auto& rr = RenderingContextStack::primary_rendering_resources();
        auto& sr = RenderingContextStack::primary_scene_node_resources();
        auto model = load_pssg(s.path, IoVerbosity::SILENT);
        try {
            auto arrays = load_pssg_arrays<TPosition, ScenePos>(
                model,
                *cfg,
                &rr,
                std::filesystem::path{ s.path }.filename().string() + '#',
                IoVerbosity::SILENT);
            load_renderable_pssg(arrays, filters, sr, added_scene_node_resources, added_instantiables);
        } catch (const std::runtime_error& e) {
            throw std::runtime_error("Error interpreting file \"" + s.path + "\": " + e.what());
        }
    }
    if (auto rv = args.arguments.try_at<std::string>(KnownArgs::resource_variable)) {
        if (args.local_json_macro_arguments == nullptr) {
            THROW_OR_ABORT("No local arguments set");
        }
        args.local_json_macro_arguments->set(*rv, added_scene_node_resources);
    }
    if (auto iv = args.arguments.try_at<std::string>(KnownArgs::instantiables_variable)) {
        if (args.local_json_macro_arguments == nullptr) {
            THROW_OR_ABORT("No local arguments set");
        }
        args.local_json_macro_arguments->set(*iv, added_instantiables);
    }
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "resource_locations",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                args.arguments.validate(KnownArgs::options);
                auto dddb = std::make_shared<DrawDistanceDb>();
                for (const auto& f : args.arguments.pathes_or_variables(KnownArgs::rw_ide_files)) {
                    dddb->add_ide(f.path);
                }
                if (args.arguments.at<bool>(KnownArgs::double_precision)) {
                    exec<CompressedScenePos>(args, dddb);
                } else {
                    exec<float>(args, dddb);
                }
            });
    }
} obj;

}
