#include "Ipl_Instances.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Geometry/Mesh/Cleanup/Cleanup_Mesh.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array_Filter.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Collision/Collidable_Mode.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Primitives.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Renderable_Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Rendering_Dynamics.hpp>
#include <Mlib/Scene_Graph/Instantiation/Instance_Information.hpp>
#include <Mlib/Scene_Graph/Instantiation/Instantiate_Frames.hpp>
#include <Mlib/Scene_Graph/Instantiation/Read_Ipl.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(files);
DECLARE_ARGUMENT(except);
DECLARE_ARGUMENT(dynamics);
DECLARE_ARGUMENT(min_vertex_distance);
}

const std::string IplInstances::key = "ipl_instances";

LoadSceneJsonUserFunction IplInstances::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    IplInstances(args.renderable_scene()).execute(args);
};

IplInstances::IplInstances(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void IplInstances::execute(const LoadSceneJsonUserFunctionArgs &args) {
    auto empty_set = std::set<std::string>();
    auto exclude = args.arguments.at_non_null<std::set<std::string>>(KnownArgs::except, empty_set);
    auto dynamics = rendering_dynamics_from_string(args.arguments.at<std::string>(KnownArgs::dynamics));
    for (const auto& file : args.arguments.pathes_or_variables(KnownArgs::files)) {
        instantiate(
            scene,
            read_ipl(file.path, dynamics),
            scene_node_resources,
            RenderingContextStack::primary_rendering_resources(),
            exclude);
    }
    {
        std::list<std::pair<TransformationMatrix<float, double, 3>, std::shared_ptr<ColoredVertexArray<float>>>> float_queue;
        std::list<std::pair<TransformationMatrix<float, double, 3>, std::shared_ptr<ColoredVertexArray<double>>>> double_queue;
        {
            static ColoredVertexArrayFilter filter{
                .included_tags = PhysicsMaterial::ATTR_COLLIDE
            };
            scene.append_static_filtered_to_queue(float_queue, double_queue, filter);
        }
        std::list<std::shared_ptr<ColoredVertexArray<double>>> hitboxes;
        {
            auto filter = PhysicsMaterial::NONE;
            auto min_vertex_distance = args.arguments.at<double>(KnownArgs::min_vertex_distance);
            auto modulo_uv = false;
            CleanupMesh<double> cleanup;
            for (const auto& [t, q] : float_queue) {
                auto cva = q->transformed<double>(t, "_ipl_float");
                cleanup(*cva, filter, min_vertex_distance, modulo_uv);
                if (!cva->empty()) {
                    hitboxes.push_back(cva);
                }
            }
            for (const auto& [t, q] : double_queue) {
                auto cva = q->transformed<double>(t, "_ipl_double");
                cleanup(*cva, filter, min_vertex_distance, modulo_uv);
                if (!cva->empty()) {
                    hitboxes.push_back(cva);
                }
            }
        }
        auto rb = rigid_cuboid(
            object_pool,
            "ipl_static",               // name
            "none",                     // asset_id
            INFINITY,                   // mass
            fixed_ones<float, 3>(),     // size
            fixed_ones<float, 3>());    // com
        rb->set_absolute_model_matrix(TransformationMatrix<float, double, 3>::identity());
        physics_engine.rigid_bodies_.add_rigid_body(*rb, {}, hitboxes, CollidableMode::STATIC);
        rb.release();
    }
}
