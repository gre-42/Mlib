#include "Create_Rigid_Disk.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Env.hpp>
#include <Mlib/Geometry/Exceptions/Polygon_Edge_Exception.hpp>
#include <Mlib/Geometry/Exceptions/Triangle_Exception.hpp>
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array_Filter.hpp>
#include <Mlib/Geometry/Mesh/Save_Polygon_To_Obj.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Collision/Collidable_Mode.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle_Flags.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Primitives.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Absolute_Movable_Setter.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
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
DECLARE_ARGUMENT(with_penetration_limits);
DECLARE_ARGUMENT(collidable_mode);
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(asset_id);
DECLARE_ARGUMENT(included_names);
DECLARE_ARGUMENT(excluded_names);
DECLARE_ARGUMENT(flags);
DECLARE_ARGUMENT(waypoint_dy);
}

CreateRigidDisk::CreateRigidDisk(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateRigidDisk::execute(const LoadSceneJsonUserFunctionArgs& args) const
{
    (*this)(CreateRigidDiskArgs{
        global_object_pool,
        args.arguments.at<std::string>(KnownArgs::node),
        args.arguments.at<std::string>(KnownArgs::name),
        args.arguments.at<std::string>(KnownArgs::asset_id),
        args.arguments.at<float>(KnownArgs::mass) * kg,
        args.arguments.at<float>(KnownArgs::radius) * meters,
        args.arguments.at<UFixedArray<float, 3>>(KnownArgs::com, fixed_zeros<float, 3>()) * meters,
        args.arguments.at<UFixedArray<float, 3>>(KnownArgs::v, fixed_zeros<float, 3>()) * kph,
        args.arguments.at<UFixedArray<float, 3>>(KnownArgs::w, fixed_zeros<float, 3>()) * rpm,
        args.arguments.at<UFixedArray<float, 3>>(KnownArgs::I_rotation, fixed_zeros<float, 3>()) * degrees,
        args.arguments.at<bool>(KnownArgs::with_penetration_limits, false),
        scene_node_resources.get_geographic_mapping("world"),
        rigid_body_vehicle_flags_from_string(args.arguments.at<std::string>(KnownArgs::flags, "none")),
        CompressedScenePos::from_float_safe(args.arguments.at<ScenePos>(KnownArgs::waypoint_dy, 0.f) * meters),
        args.arguments.try_at_non_null(KnownArgs::hitboxes),
        ColoredVertexArrayFilter{
            .included_tags = PhysicsMaterial::ATTR_COLLIDE,
            .included_names = Mlib::compile_regex(args.arguments.at<std::string>(KnownArgs::included_names, "")),
            .excluded_names = Mlib::compile_regex(args.arguments.at<std::string>(KnownArgs::excluded_names, "$ ^"))},
        collidable_mode_from_string(args.arguments.at<std::string>(KnownArgs::collidable_mode))});
}

RigidBodyVehicle& CreateRigidDisk::operator () (const CreateRigidDiskArgs& args) const
{
    auto pl = physics_engine.config().penetration_limits();
    auto rb = rigid_disk(
        global_object_pool,
        args.name,
        args.asset_id,
        args.mass,
        args.radius,
        args.com,
        args.v,
        args.w,
        args.I_rotation,
        args.with_penetration_limits ? &pl : nullptr,
        args.geographic_coordinates);
    rb->flags_ = args.flags;
    rb->set_waypoint_ofs(args.waypoint_dy);
    std::list<std::shared_ptr<ColoredVertexArray<float>>> s_hitboxes;
    std::list<std::shared_ptr<ColoredVertexArray<CompressedScenePos>>> d_hitboxes;
    std::list<TypedMesh<std::shared_ptr<IIntersectable>>> intersectables;
    if (args.hitboxes.has_value()) {
        {
            auto acva = scene_node_resources.get_arrays(*args.hitboxes, args.hitbox_filter);
            auto insert = [](auto& hitboxes, const auto& cvas){
                for (const auto& cva: cvas) {
                    hitboxes.push_back(cva);
                }
            };
            insert(s_hitboxes, acva->scvas);
            insert(d_hitboxes, acva->dcvas);
        }
        intersectables = scene_node_resources.get_intersectables(*args.hitboxes);
    }
    // 1. Set movable, which updates the transformation-matrix.
    auto& result = *rb;
    AbsoluteMovableSetter ams{ scene.get_node(args.node, DP_LOC), std::move(rb), CURRENT_SOURCE_LOCATION };
    // 2. Add to physics engine.
    try {
        physics_engine.rigid_bodies_.add_rigid_body(
            *ams.absolute_movable,
            s_hitboxes,
            d_hitboxes,
            intersectables,
            args.collidable_mode);
        ams.absolute_movable.release();
    } catch (const TriangleException<double>& e) {
        if (auto filename = try_getenv("RIGID_BODY_TRIANGLE_FILENAME"); filename.has_value()) {
            save_triangle_to_obj(*filename, {e.a, e.b, e.c});
        }
        throw std::runtime_error(e.str("Error", scene_node_resources.get_geographic_mapping("world")));
    } catch (const PolygonEdgeException<double, 3>& e) {
        if (auto filename = try_getenv("RIGID_BODY_TRIANGLE_FILENAME"); filename.has_value()) {
            save_triangle_to_obj(*filename, e.poly);
        }
        throw std::runtime_error(e.str("Error", scene_node_resources.get_geographic_mapping("world")));
    } catch (const PolygonEdgeException<double, 4>& e) {
        if (auto filename = try_getenv("RIGID_BODY_TRIANGLE_FILENAME"); filename.has_value()) {
            save_quad_to_obj(*filename, e.poly);
        }
        throw std::runtime_error(e.str("Error", scene_node_resources.get_geographic_mapping("world")));
    }
    return result;
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "rigid_disk",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                args.arguments.validate(KnownArgs::options);
                CreateRigidDisk(args.renderable_scene()).execute(args);
            });
    }
} obj;

}
