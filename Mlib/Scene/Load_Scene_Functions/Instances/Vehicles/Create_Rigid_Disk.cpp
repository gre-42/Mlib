#include "Create_Rigid_Disk.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
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
DECLARE_ARGUMENT(collidable_mode);
DECLARE_ARGUMENT(name);
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
        args.arguments.at<float>(KnownArgs::mass) * kg,
        args.arguments.at<float>(KnownArgs::radius) * meters,
        args.arguments.at<FixedArray<float, 3>>(KnownArgs::com, fixed_zeros<float, 3>()) * meters,
        args.arguments.at<FixedArray<float, 3>>(KnownArgs::v, fixed_zeros<float, 3>()) * meters / s,
        args.arguments.at<FixedArray<float, 3>>(KnownArgs::w, fixed_zeros<float, 3>()) * degrees / s,
        scene_node_resources.get_geographic_mapping("world"));
    if (args.arguments.contains(KnownArgs::flags)) {
        rb->flags_ = rigid_body_vehicle_flags_from_string(args.arguments.at<std::string>(KnownArgs::flags));
    }
    std::list<std::shared_ptr<ColoredVertexArray<float>>> s_hitboxes;
    std::list<std::shared_ptr<ColoredVertexArray<double>>> d_hitboxes;
    if (args.arguments.contains(KnownArgs::hitboxes)) {
        auto hitboxes = args.arguments.at<std::string>(KnownArgs::hitboxes);
        auto cvas = scene_node_resources.get_animated_arrays(hitboxes);
        s_hitboxes.insert(s_hitboxes.end(), cvas->scvas.begin(), cvas->scvas.end());
        d_hitboxes.insert(d_hitboxes.end(), cvas->dcvas.begin(), cvas->dcvas.end());
    }
    CollidableMode collidable_mode = collidable_mode_from_string(args.arguments.at<std::string>(KnownArgs::collidable_mode));
    // 1. Set movable, which updates the transformation-matrix.
    AbsoluteMovableSetter ams{scene.get_node(args.arguments.at<std::string>(KnownArgs::node)), std::move(rb)};
    // 2. Add to physics engine.
    physics_engine.rigid_bodies_.add_rigid_body(
        std::move(ams.absolute_movable),
        s_hitboxes,
        d_hitboxes,
        collidable_mode,
        PhysicsResourceFilter{
            .cva_filter = {
                .included_names = Mlib::compile_regex(args.arguments.at<std::string>(KnownArgs::included_names, "")),
                .excluded_names = Mlib::compile_regex(args.arguments.at<std::string>(KnownArgs::excluded_names, "$ ^"))}});
}
