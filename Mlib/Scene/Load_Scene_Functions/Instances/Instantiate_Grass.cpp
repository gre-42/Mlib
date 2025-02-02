#include "Instantiate_Grass.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Render/Renderables/Triangle_Sampler/Resource_Name_Cycle.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Instantiation/Read_Grs.hpp>
#include <Mlib/Scene_Graph/Instantiation/Root_Instantiation_Options.hpp>
#include <Mlib/Scene_Graph/Resources/Batch_Resource_Instantiator.hpp>
#include <Mlib/Scene_Graph/Resources/Parsed_Resource_Name.hpp>
#include <Mlib/Scene_Graph/Resources/Renderable_Resource_Filter.hpp>
#include <Mlib/Strings/Filesystem_Path.hpp>
#include <Mlib/Threads/Thread_Top.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(filenames);
DECLARE_ARGUMENT(resources);
DECLARE_ARGUMENT(height_tolerance);
}

const std::string InstantiateGrass::key = "instantiate_grass";

LoadSceneJsonUserFunction InstantiateGrass::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    InstantiateGrass(args.renderable_scene()).execute(args);
};

InstantiateGrass::InstantiateGrass(RenderableScene& renderable_scene) 
    : LoadSceneInstanceFunction{ renderable_scene }
{}

void InstantiateGrass::execute(const LoadSceneJsonUserFunctionArgs &args) {
    auto height_tolerance = args.arguments.at<ScenePos>(KnownArgs::height_tolerance) * meters;
    BatchResourceInstantiator bri;
    auto parse_resource_name_func = [this](const std::string& jma){
        return parse_resource_name(scene_node_resources, jma);
    };
    auto resource_names = args.arguments.at_vector<std::string>(KnownArgs::resources, parse_resource_name_func);
    ResourceNameCycle rnc{ resource_names };
    for (const auto& fpath : args.arguments.try_pathes_or_variables(KnownArgs::filenames)) {
        FunctionGuard fg{ "Load \"" + short_path(fpath.path) + '"' };
        auto model = Grs::load_grs(
            fpath.path,
            IoVerbosity::SILENT);
        for (const auto& cell : model.cells) {
            for (const auto& p16 : cell.coords16) {
                if (auto prn = rnc.try_multiple_times(10); prn != nullptr) {
                    auto p16s = p16.p;
                    std::swap(p16s(1), p16s(2));
                    auto p =
                        cell.aabb.min.casted<ScenePos>() +
                        cell.aabb.size().casted<ScenePos>() *
                        p16s.casted<ScenePos>() / double{ UINT16_MAX };
                    FixedArray<ScenePos, 3> intersection_point = uninitialized;
                    if (physics_engine.collision_query_.can_see(
                        p - FixedArray<ScenePos, 3>{0.f, height_tolerance, 0.f},
                        p + FixedArray<ScenePos, 3>{0.f, height_tolerance, 0.f},
                        nullptr,    // excluded0
                        nullptr,    // excluded1
                        true,       // only_terrain
                        PhysicsMaterial::OBJ_BULLET_COLLIDABLE_MASK,
                        &intersection_point))
                    {
                        continue;
                    }
                    bri.add_parsed_resource_name(intersection_point.casted<CompressedScenePos>(), *prn, 0.f, 1.f);
                }
            }
            for (const auto& p8 : cell.coords8) {
                continue;
                if (auto prn = rnc.try_multiple_times(10); prn != nullptr) {
                    auto p =
                        cell.aabb.min.casted<double>() +
                        cell.aabb.size().casted<double>() *
                        p8.p.casted<double>() / double{ UINT8_MAX };
                    bri.add_parsed_resource_name(p.casted<CompressedScenePos>(), *prn, 0.f, 1.f);
                }
            }
        }
    }
    FunctionGuard fg{ "Instantiate grass" };
    bri.instantiate_root_renderables(
        scene_node_resources,
        RootInstantiationOptions{
            .rendering_resources = &RenderingContextStack::primary_rendering_resources(),
            .imposters = nullptr,
            .supply_depots = nullptr,
            .instance_name = VariableAndHash<std::string>{ "grass_world" },
            .absolute_model_matrix = TransformationMatrix<float, ScenePos, 3>::identity(),
            .scene = scene,
            .renderable_resource_filter = RenderableResourceFilter{}
    });
}
