#include "Create_Rigid_Cuboid.hpp"
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Physics/Collision/Collidable_Mode.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Primitives.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Absolute_Movable_Setter.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Physics_Resource_Filter.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
#include <Mlib/Strings/String.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(NODE);
DECLARE_OPTION(HITBOXES);
DECLARE_OPTION(MASS);
DECLARE_OPTION(SIZE_X);
DECLARE_OPTION(SIZE_Y);
DECLARE_OPTION(SIZE_Z);
DECLARE_OPTION(COM_X);
DECLARE_OPTION(COM_Y);
DECLARE_OPTION(COM_Z);
DECLARE_OPTION(V_X);
DECLARE_OPTION(V_Y);
DECLARE_OPTION(V_Z);
DECLARE_OPTION(W_X);
DECLARE_OPTION(W_Y);
DECLARE_OPTION(W_Z);
DECLARE_OPTION(COLLIDABLE_MODE);
DECLARE_OPTION(NAME);
DECLARE_OPTION(INCLUDE);
DECLARE_OPTION(EXCLUDE);

LoadSceneUserFunction CreateRigidCuboid::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*rigid_cuboid"
        "\\s+node=([\\w+-.]+)"
        "(?:\\s+hitboxes=([\\w+-. \\(\\)/]+))?"
        "\\s+mass=([\\w+-.]+)"
        "\\s+size=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "(?:\\s+com=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+))?"
        "(?:\\s+v=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+))?"
        "(?:\\s+w=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+))?"
        "\\s+collidable_mode=(terrain|small_static|small_moving)"
        "(?:\\s+name=([\\w+-.]+))?"
        "(?:\\s+included_names=(.*?))?"
        "(?:\\s+excluded_names=(.*?))?$");
    Mlib::re::smatch match;
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
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    std::unique_ptr<RigidBodyVehicle> rb = rigid_cuboid(
        match[NAME].str(),
        safe_stof(match[MASS].str()) * kg,
        FixedArray<float, 3>{
            safe_stof(match[SIZE_X].str()) * meters,
            safe_stof(match[SIZE_Y].str()) * meters,
            safe_stof(match[SIZE_Z].str()) * meters},
        FixedArray<float, 3>{
            match[COM_X].str().empty() ? 0.f : safe_stof(match[COM_X].str()) * meters,
            match[COM_Y].str().empty() ? 0.f : safe_stof(match[COM_Y].str()) * meters,
            match[COM_Z].str().empty() ? 0.f : safe_stof(match[COM_Z].str()) * meters},
        FixedArray<float, 3>{
            match[V_X].str().empty() ? 0.f : safe_stof(match[V_X].str()) * meters / s,
            match[V_Y].str().empty() ? 0.f : safe_stof(match[V_Y].str()) * meters / s,
            match[V_Z].str().empty() ? 0.f : safe_stof(match[V_Z].str()) * meters / s},
        FixedArray<float, 3>{
            match[W_X].str().empty() ? 0.f : safe_stof(match[W_X].str()) * degrees / s,
            match[W_Y].str().empty() ? 0.f : safe_stof(match[W_Y].str()) * degrees / s,
            match[W_Z].str().empty() ? 0.f : safe_stof(match[W_Z].str()) * degrees / s},
        scene_node_resources.get_geographic_mapping("world"));
    std::list<std::shared_ptr<ColoredVertexArray<float>>> s_hitboxes;
    std::list<std::shared_ptr<ColoredVertexArray<double>>> d_hitboxes;
    if (match[HITBOXES].matched) {
        for (const auto& s : string_to_list(match[HITBOXES].str())) {
            {
                auto& cvas = scene_node_resources.get_animated_arrays(s)->scvas;
                s_hitboxes.insert(s_hitboxes.end(), cvas.begin(), cvas.end());
            }
            {
                auto& cvas = scene_node_resources.get_animated_arrays(s)->dcvas;
                d_hitboxes.insert(d_hitboxes.end(), cvas.begin(), cvas.end());
            }
        }
    }
    CollidableMode collidable_mode = collidable_mode_from_string(match[COLLIDABLE_MODE].str());
    // 1. Set movable, which updates the transformation-matrix.
    AbsoluteMovableSetter ams{scene.get_node(match[NODE].str()), std::move(rb)};
    // 2. Add to physics engine.
    physics_engine.rigid_bodies_.add_rigid_body(
        std::move(ams.absolute_movable),
        s_hitboxes,
        d_hitboxes,
        collidable_mode,
        PhysicsResourceFilter{
            .cva_filter = {
                .included_names = Mlib::compile_regex(match[INCLUDE].str()),
                .excluded_names = Mlib::compile_regex(
                    match[EXCLUDE].matched
                        ? match[EXCLUDE].str()
                        : "$ ^")}});
}
