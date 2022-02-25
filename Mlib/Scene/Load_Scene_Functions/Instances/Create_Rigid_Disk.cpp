#include "Create_Rigid_Disk.hpp"
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Physics/Collision/Collidable_Mode.hpp>
#include <Mlib/Physics/Containers/Rigid_Body_Resource_Filter.hpp>
#include <Mlib/Physics/Physics_Engine.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Primitives.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>
#include <Mlib/Scene_Graph/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(NODE);
DECLARE_OPTION(HITBOX);
DECLARE_OPTION(TIRELINES);
DECLARE_OPTION(GRIND_CONTACTS);
DECLARE_OPTION(GRIND_LINES);
DECLARE_OPTION(ALIGNMENT_CONTACTS);
DECLARE_OPTION(ALIGNMENT_PLANES);
DECLARE_OPTION(MASS);
DECLARE_OPTION(RADIUS);
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

LoadSceneUserFunction CreateRigidDisk::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*rigid_disk"
        "\\s+node=([\\w+-.]+)"
        "(?:\\s+hitbox=([\\w-. \\(\\)/+-]+))?"
        "(?:\\s+tirelines=([\\w-. \\(\\)/+-]+))?"
        "(?:\\s+grind_contacts=([\\w-. \\(\\)/+-]+))?"
        "(?:\\s+grind_lines=([\\w-. \\(\\)/+-]+))?"
        "(?:\\s+alignment_contacts=([\\w-. \\(\\)/+-]+))?"
        "(?:\\s+alignment_planes=([\\w-. \\(\\)/+-]+))?"
        "\\s+mass=([\\w+-.]+)"
        "\\s+radius=([\\w+-.]+)"
        "(?:\\s+com=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+))?"
        "(?:\\s+v=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+))?"
        "(?:\\s+w=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+))?"
        "\\s+collidable_mode=(terrain|small_static|small_moving)"
        "(?:\\s+name=([\\w+-.]+))?"
        "(?:\\s+include=(.*?))?"
        "(?:\\s+exclude=(.*?))?$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        CreateRigidDisk(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

CreateRigidDisk::CreateRigidDisk(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateRigidDisk::execute(
    const std::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    std::shared_ptr<RigidBodyVehicle> rb = rigid_disk(
        safe_stof(match[MASS].str()),
        safe_stof(match[RADIUS].str()),
        FixedArray<float, 3>{
            match[COM_X].str().empty() ? 0.f : safe_stof(match[COM_X].str()),
            match[COM_Y].str().empty() ? 0.f : safe_stof(match[COM_Y].str()),
            match[COM_Z].str().empty() ? 0.f : safe_stof(match[COM_Z].str())},
        FixedArray<float, 3>{
            match[V_X].str().empty() ? 0.f : safe_stof(match[V_X].str()),
            match[V_Y].str().empty() ? 0.f : safe_stof(match[V_Y].str()),
            match[V_Z].str().empty() ? 0.f : safe_stof(match[V_Z].str())},
        FixedArray<float, 3>{
            match[W_X].str().empty() ? 0.f : safe_stof(match[W_X].str()) * float(M_PI / 180),
            match[W_Y].str().empty() ? 0.f : safe_stof(match[W_Y].str()) * float(M_PI / 180),
            match[W_Z].str().empty() ? 0.f : safe_stof(match[W_Z].str()) * float(M_PI / 180)},
        scene_node_resources.get_geographic_mapping("world"),
        match[NAME].str());
    std::list<std::shared_ptr<ColoredVertexArray>> hitbox;
    if (match[HITBOX].matched) {
        hitbox = scene_node_resources.get_animated_arrays(match[HITBOX].str())->cvas;
    }
    std::list<std::shared_ptr<ColoredVertexArray>> tirelines;
    if (match[TIRELINES].matched) {
        tirelines = scene_node_resources.get_animated_arrays(match[TIRELINES].str())->cvas;
    }
    std::list<std::shared_ptr<ColoredVertexArray>> grind_contacts;
    if (match[GRIND_CONTACTS].matched) {
        grind_contacts = scene_node_resources.get_animated_arrays(match[GRIND_CONTACTS].str())->cvas;
    }
    std::list<std::shared_ptr<ColoredVertexArray>> grind_lines;
    if (match[GRIND_LINES].matched) {
        grind_lines = scene_node_resources.get_animated_arrays(match[GRIND_LINES].str())->cvas;
    }
    std::list<std::shared_ptr<ColoredVertexArray>> alignment_contacts;
    if (match[ALIGNMENT_CONTACTS].matched) {
        alignment_contacts = scene_node_resources.get_animated_arrays(match[ALIGNMENT_CONTACTS].str())->cvas;
    }
    std::list<std::shared_ptr<ColoredVertexArray>> alignment_planes;
    if (match[ALIGNMENT_PLANES].matched) {
        alignment_planes = scene_node_resources.get_animated_arrays(match[ALIGNMENT_PLANES].str())->cvas;
    }
    CollidableMode collidable_mode = collidable_mode_from_string(match[COLLIDABLE_MODE].str());
    // 1. Set movable, which updates the transformation-matrix.
    scene.get_node(match[NODE].str()).set_absolute_movable(rb.get());
    // 2. Add to physics engine.
    physics_engine.rigid_bodies_.add_rigid_body(
        rb,
        hitbox,
        tirelines,
        grind_contacts,
        grind_lines,
        alignment_contacts,
        alignment_planes,
        collidable_mode,
        RigidBodyResourceFilter{
            .include = Mlib::compile_regex(match[INCLUDE].str()),
            .exclude = Mlib::compile_regex(
                match[EXCLUDE].matched
                    ? match[EXCLUDE].str()
                    : "$ ^")});
}
