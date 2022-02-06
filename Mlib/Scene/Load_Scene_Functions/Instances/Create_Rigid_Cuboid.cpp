#include "Create_Rigid_Cuboid.hpp"
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Physics/Collision/Collidable_Mode.hpp>
#include <Mlib/Physics/Physics_Engine.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Primitives.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>
#include <Mlib/Scene_Graph/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>

using namespace Mlib;

LoadSceneUserFunction CreateRigidCuboid::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*rigid_cuboid"
        "\\s+node=([\\w+-.]+)"
        "\\s+hitbox=([\\w-. \\(\\)/+-]+)"
        "(?:\\s+tirelines=([\\w-. \\(\\)/+-]+))?"
        "\\s+mass=([\\w+-.]+)"
        "\\s+size=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "(?:\\s+com=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+))?"
        "(?:\\s+v=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+))?"
        "(?:\\s+w=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+))?"
        "\\s+collidable_mode=(terrain|small_static|small_moving)"
        "(?:\\s+name=([\\w+-.]+))?$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        CreateRigidCuboid(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

CreateRigidCuboid::CreateRigidCuboid(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateRigidCuboid::execute(
    const std::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    std::shared_ptr<RigidBodyVehicle> rb = rigid_cuboid(
        physics_engine.rigid_bodies_,
        safe_stof(match[4].str()),
        FixedArray<float, 3>{
            safe_stof(match[5].str()),
            safe_stof(match[6].str()),
            safe_stof(match[7].str())},
        FixedArray<float, 3>{
            match[8].str().empty() ? 0.f : safe_stof(match[8].str()),
            match[9].str().empty() ? 0.f : safe_stof(match[9].str()),
            match[10].str().empty() ? 0.f : safe_stof(match[10].str())},
        FixedArray<float, 3>{
            match[11].str().empty() ? 0.f : safe_stof(match[11].str()),
            match[12].str().empty() ? 0.f : safe_stof(match[12].str()),
            match[13].str().empty() ? 0.f : safe_stof(match[13].str())},
        FixedArray<float, 3>{
            match[14].str().empty() ? 0.f : safe_stof(match[14].str()) * float(M_PI / 180),
            match[15].str().empty() ? 0.f : safe_stof(match[15].str()) * float(M_PI / 180),
            match[16].str().empty() ? 0.f : safe_stof(match[16].str()) * float(M_PI / 180)},
        scene_node_resources.get_geographic_mapping("world"),
        match[18].str());
    std::list<std::shared_ptr<ColoredVertexArray>> hitbox = scene_node_resources.get_animated_arrays(match[2].str())->cvas;
    std::list<std::shared_ptr<ColoredVertexArray>> tirelines;
    if (!match[3].str().empty()) {
        tirelines = scene_node_resources.get_animated_arrays(match[3].str())->cvas;
    }
    CollidableMode collidable_mode = collidable_mode_from_string(match[17].str());
    // 1. Set movable, which updates the transformation-matrix
    scene.get_node(match[1].str())->set_absolute_movable(rb.get());
    // 2. Add to physics engine. This should not fail,
    //    i.e. all parsing is already done.
    physics_engine.rigid_bodies_.add_rigid_body(
        rb,
        hitbox,
        tirelines,
        collidable_mode);
}
