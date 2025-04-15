#include "Instantiate_Statics.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Geometry/Mesh/Cleanup/Cleanup_Mesh.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array_Filter.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Physics/Collision/Collidable_Mode.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Primitives.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Threads/Thread_Top.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(min_vertex_distance);
}

InstantiateStatics::InstantiateStatics(RenderableScene& renderable_scene) 
    : LoadSceneInstanceFunction{ renderable_scene }
{}

void InstantiateStatics::execute(const LoadSceneJsonUserFunctionArgs &args) {
    std::list<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<ColoredVertexArray<float>>>> float_queue;
    std::list<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<ColoredVertexArray<CompressedScenePos>>>> double_queue;
    scene.append_physics_to_queue(float_queue, double_queue);
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
            FunctionGuard fg{ "Instantiate \"" + q->name.full_name() + '"' };
            auto cva = q->transformed<CompressedScenePos>(t, "_ipl_float");
            cleanup(*cva, filter, min_vertex_distance, modulo_uv);
            if (!cva->empty()) {
                add_rigid_cuboid(cva, "ipl_static_float_" + q->name.full_name());
            }
        }
        for (const auto& [t, q] : double_queue) {
            FunctionGuard fg{ "Instantiate \"" + q->name.full_name() + '"' };
            auto cva = q->transformed<CompressedScenePos>(t, "_ipl_double");
            cleanup(*cva, filter, min_vertex_distance, modulo_uv);
            if (!cva->empty()) {
                add_rigid_cuboid(cva, "ipl_static_double_" + q->name.full_name());
            }
        }
    }
}

namespace {

static struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "instantiate_statics",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                args.arguments.validate(KnownArgs::options);
                InstantiateStatics(args.renderable_scene()).execute(args);
            });
    }
} obj;

}
