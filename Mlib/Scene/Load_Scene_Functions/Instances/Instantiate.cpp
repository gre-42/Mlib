#include "Instantiate.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Geometry/Instance/Instance_Information.hpp>
#include <Mlib/Geometry/Instance/Rendering_Dynamics.hpp>
#include <Mlib/Geometry/Mesh/Cleanup/Cleanup_Mesh.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array_Filter.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Collision/Collidable_Mode.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Primitives.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
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
DECLARE_ARGUMENT(min_vertex_distance);
DECLARE_ARGUMENT(instantiated_resources);
}

const std::string Instantiate::key = "instantiate";

LoadSceneJsonUserFunction Instantiate::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    Instantiate(args.renderable_scene()).execute(args);
};

Instantiate::Instantiate(RenderableScene& renderable_scene) 
    : LoadSceneInstanceFunction{ renderable_scene }
{}

void Instantiate::execute(const LoadSceneJsonUserFunctionArgs &args) {
    auto empty_set = std::set<std::string>();
    auto matching_set = std::set<std::string>{""};
    auto required_prefixes = args.arguments.at<std::set<std::string>>(KnownArgs::required_prefixes, matching_set);
    auto exclude = args.arguments.at_non_null<std::set<std::string>>(KnownArgs::except, empty_set);
    auto dynamics = rendering_dynamics_from_string(args.arguments.at<std::string>(KnownArgs::dynamics));
    auto ir = args.arguments.try_at<std::string>(KnownArgs::instantiated_resources);
    std::set<std::string> instantiated;
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
                ir.has_value() ? &instantiated : nullptr);
        }
    }
    for (const auto& name : args.arguments.try_at_vector<std::string>(KnownArgs::instantiables)) {
        FunctionGuard fg{ "Instantiate \"" + name + '"' };
        instantiate(
            scene,
            scene_node_resources.instantiable(name),
            scene_node_resources,
            rendering_resources,
            required_prefixes,
            exclude,
            ir.has_value() ? &instantiated : nullptr);
    }
    if (ir.has_value()) {
        if (args.local_json_macro_arguments == nullptr) {
            THROW_OR_ABORT("instantiated_resources requires local arguments");
        }
        args.local_json_macro_arguments->set(*ir, instantiated);
    }
    {
        std::list<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<ColoredVertexArray<float>>>> float_queue;
        std::list<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<ColoredVertexArray<CompressedScenePos>>>> double_queue;
        {
            static ColoredVertexArrayFilter filter{
                .included_tags = PhysicsMaterial::ATTR_COLLIDE
            };
            scene.append_static_filtered_to_queue(float_queue, double_queue, filter);
        }
        auto add_rigid_cuboid = [&](const auto& cva, const std::string& name){
            auto rb = rigid_cuboid(
                object_pool,
                name,                       // name
                "none",                     // asset_id
                INFINITY,                   // mass
                fixed_ones<float, 3>(),     // size
                fixed_ones<float, 3>());    // com
            rb->set_absolute_model_matrix(TransformationMatrix<float, ScenePos, 3>::identity());
            physics_engine.rigid_bodies_.add_rigid_body(*rb, {}, { cva }, {}, CollidableMode::STATIC);
            rb.release();
            };
        {
            auto filter = PhysicsMaterial::NONE;
            auto min_vertex_distance = args.arguments.at<CompressedScenePos>(KnownArgs::min_vertex_distance);
            auto modulo_uv = false;
            CleanupMesh<CompressedScenePos> cleanup;
            for (const auto& [t, q] : float_queue) {
                FunctionGuard fg{ "Instantiate \"" + q->name + '"' };
                auto cva = q->transformed<CompressedScenePos>(t, "_ipl_float");
                cleanup(*cva, filter, min_vertex_distance, modulo_uv);
                if (!cva->empty()) {
                    add_rigid_cuboid(cva, "ipl_static_float");
                }
            }
            for (const auto& [t, q] : double_queue) {
                FunctionGuard fg{ "Instantiate \"" + q->name + '"' };
                auto cva = q->transformed<CompressedScenePos>(t, "_ipl_double");
                cleanup(*cva, filter, min_vertex_distance, modulo_uv);
                if (!cva->empty()) {
                    add_rigid_cuboid(cva, "ipl_static_double");
                }
            }
        }
    }
}
