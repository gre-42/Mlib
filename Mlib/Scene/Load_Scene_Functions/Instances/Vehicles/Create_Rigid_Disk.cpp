#include "Create_Rigid_Disk.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Env.hpp>
#include <Mlib/Geometry/Exceptions/Triangle_Edge_Exception.hpp>
#include <Mlib/Geometry/Exceptions/Triangle_Exception.hpp>
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Save_Triangle_To_Obj.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Collision/Collidable_Mode.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle_Flags.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Primitives.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Absolute_Movable_Setter.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Resources/Physics_Resource_Filter.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Strings/String.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(node);
DECLARE_ARGUMENT(hitboxes);
DECLARE_ARGUMENT(mass);
DECLARE_ARGUMENT(radius);
DECLARE_ARGUMENT(com);
DECLARE_ARGUMENT(v);
DECLARE_ARGUMENT(w);
DECLARE_ARGUMENT(I_rotation);
DECLARE_ARGUMENT(collidable_mode);
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(asset_id);
DECLARE_ARGUMENT(included_names);
DECLARE_ARGUMENT(excluded_names);
DECLARE_ARGUMENT(flags);
}

const std::string CreateRigidDisk::key = "rigid_disk";

LoadSceneJsonUserFunction CreateRigidDisk::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    CreateRigidDisk(args.renderable_scene()).execute(args);
};

CreateRigidDisk::CreateRigidDisk(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateRigidDisk::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    std::unique_ptr<RigidBodyVehicle> rb = rigid_disk(
        args.arguments.at<std::string>(KnownArgs::name),
        args.arguments.at<std::string>(KnownArgs::asset_id),
        args.arguments.at<float>(KnownArgs::mass) * kg,
        args.arguments.at<float>(KnownArgs::radius) * meters,
        args.arguments.at<FixedArray<float, 3>>(KnownArgs::com, fixed_zeros<float, 3>()) * meters,
        args.arguments.at<FixedArray<float, 3>>(KnownArgs::v, fixed_zeros<float, 3>()) * meters / s,
        args.arguments.at<FixedArray<float, 3>>(KnownArgs::w, fixed_zeros<float, 3>()) * degrees / s,
        args.arguments.at<FixedArray<float, 3>>(KnownArgs::I_rotation, fixed_zeros<float, 3>()) * degrees,
        scene_node_resources.get_geographic_mapping("world"));
    if (args.arguments.contains(KnownArgs::flags)) {
        rb->flags_ = rigid_body_vehicle_flags_from_string(args.arguments.at<std::string>(KnownArgs::flags));
    }
    std::list<std::shared_ptr<ColoredVertexArray<float>>> s_hitboxes;
    std::list<std::shared_ptr<ColoredVertexArray<double>>> d_hitboxes;
    if (args.arguments.contains_non_null(KnownArgs::hitboxes)) {
        PhysicsResourceFilter filter{
            .cva_filter = {
                .included_names = Mlib::compile_regex(args.arguments.at<std::string>(KnownArgs::included_names, "")),
                .excluded_names = Mlib::compile_regex(args.arguments.at<std::string>(KnownArgs::excluded_names, "$ ^"))}};
        auto acva = scene_node_resources.get_physics_arrays(
            args.arguments.at<std::string>(KnownArgs::hitboxes));
        auto insert = [&filter](auto& hitboxes, const auto& cvas){
            for (const auto& cva: cvas) {
                if (filter.matches(*cva)) {
                    hitboxes.push_back(cva);
                }
            }
        };
        insert(s_hitboxes, acva->scvas);
        insert(d_hitboxes, acva->dcvas);
    }
    CollidableMode collidable_mode = collidable_mode_from_string(args.arguments.at<std::string>(KnownArgs::collidable_mode));
    // 1. Set movable, which updates the transformation-matrix.
    AbsoluteMovableSetter ams{scene.get_node(args.arguments.at<std::string>(KnownArgs::node), DP_LOC), std::move(rb)};
    // 2. Add to physics engine.
    try {
        physics_engine.rigid_bodies_.add_rigid_body(
            std::move(ams.absolute_movable),
            s_hitboxes,
            d_hitboxes,
            collidable_mode);
    } catch (const TriangleException<double>& e) {
        if (auto filename = try_getenv("RIGID_BODY_TRIANGLE_FILENAME"); filename.has_value()) {
            save_triangle_to_obj(filename.value(), {e.a, e.b, e.c});
        }
        throw std::runtime_error(e.str("Error", scene_node_resources.get_geographic_mapping("world")));
    } catch (const TriangleEdgeException<double>& e) {
        if (auto filename = try_getenv("RIGID_BODY_TRIANGLE_FILENAME"); filename.has_value()) {
            save_triangle_to_obj(filename.value(), {e.a, e.b, e.c});
        }
        throw std::runtime_error(e.str("Error", scene_node_resources.get_geographic_mapping("world")));
    }
}
