#include "Instantiate.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Geometry/Instance/Instance_Information.hpp>
#include <Mlib/Geometry/Instance/Rendering_Dynamics.hpp>
#include <Mlib/Geometry/Mesh/Cleanup/Cleanup_Mesh.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array_Filter.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene/Renderable_Scene.hpp>
#include <Mlib/Scene_Graph/Instantiation/Instantiate_Frames.hpp>
#include <Mlib/Scene_Graph/Instantiation/Read_Ipl.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Strings/Filesystem_Path.hpp>
#include <Mlib/Threads/Thread_Top.hpp>
#include <set>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(ipl_files);
DECLARE_ARGUMENT(instantiables);
DECLARE_ARGUMENT(required_prefixes);
DECLARE_ARGUMENT(except);
DECLARE_ARGUMENT(dynamics);
DECLARE_ARGUMENT(instantiated_resources);
DECLARE_ARGUMENT(instantiated_root_nodes);
}

Instantiate::Instantiate(PhysicsScene& physics_scene) 
    : LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void Instantiate::execute(const LoadSceneJsonUserFunctionArgs &args) {
    args.arguments.validate(KnownArgs::options);
    auto empty_set = std::set<VariableAndHash<std::string>>();
    auto matching_set = std::set<std::string>{""};
    auto required_prefixes = args.arguments.at<std::set<std::string>>(KnownArgs::required_prefixes, matching_set);
    auto exclude = args.arguments.at_non_null<std::set<VariableAndHash<std::string>>>(KnownArgs::except, empty_set);
    auto dynamics = rendering_dynamics_from_string(args.arguments.at<std::string>(KnownArgs::dynamics));
    auto ires = args.arguments.try_at<std::string>(KnownArgs::instantiated_resources);
    auto irno = args.arguments.try_at<std::string>(KnownArgs::instantiated_root_nodes);
    std::set<VariableAndHash<std::string>> instantiated_resources;
    std::list<VariableAndHash<std::string>> instantiated_root_nodes;
    for (const auto& file : args.arguments.try_pathes_or_variables(KnownArgs::ipl_files)) {
        FunctionGuard fg{ "Instantiate \"" + short_path(file.path) + '"'};
        for (const auto& info : read_ipl(file.path, dynamics)) {
            instantiate(
                scene,
                info,
                scene_node_resources,
                rendering_resources,
                required_prefixes,
                exclude,
                ires.has_value() ? &instantiated_resources : nullptr,
                irno.has_value() ? &instantiated_root_nodes : nullptr);
        }
    }
    for (const auto& name : args.arguments.try_at_vector<VariableAndHash<std::string>>(KnownArgs::instantiables)) {
        FunctionGuard fg{ "Instantiate \"" + *name + '"' };
        instantiate(
            scene,
            scene_node_resources.instantiable(name),
            scene_node_resources,
            rendering_resources,
            required_prefixes,
            exclude,
            ires.has_value() ? &instantiated_resources : nullptr,
            irno.has_value() ? &instantiated_root_nodes : nullptr);
    }
    if (ires.has_value()) {
        if (args.local_json_macro_arguments == nullptr) {
            THROW_OR_ABORT("instantiated_resources requires local arguments");
        }
        args.local_json_macro_arguments->set(*ires, instantiated_resources);
    }
    if (irno.has_value()) {
        if (args.local_json_macro_arguments == nullptr) {
            THROW_OR_ABORT("instantiated_root_nodes requires local arguments");
        }
        args.local_json_macro_arguments->set(*irno, instantiated_root_nodes);
    }
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "instantiate",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                Instantiate(args.physics_scene()).execute(args);
            });
    }
} obj;

}
