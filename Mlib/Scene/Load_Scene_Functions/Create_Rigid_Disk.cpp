#include "Create_Rigid_Disk.hpp"
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Physics/Collision/Collidable_Mode.hpp>
#include <Mlib/Physics/Physics_Engine.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Primitives.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>

using namespace Mlib;

LoadSceneInstanceFunction::UserFunction CreateRigidDisk::user_function = [](
    const std::string& line,
    const std::function<RenderableScene&()>& renderable_scene,
    const std::function<FPath(const std::string&)>& fpath,
    const MacroLineExecutor& macro_line_executor,
    SubstitutionMap& external_substitutions,
    SubstitutionMap* local_substitutions,
    RegexSubstitutionCache& rsc)
{
    static DECLARE_REGEX(regex,
        "^\\s*rigid_disk"
        "\\s+node=([\\w+-.]+)"
        "\\s+hitbox=([\\w-. \\(\\)/+-]+)"
        "(?:\\s+tirelines=([\\w-. \\(\\)/+-]+))?"
        "\\s+mass=([\\w+-.]+)"
        "\\s+radius=([\\w+-.]+)"
        "(?:\\s+com=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+))?"
        "(?:\\s+v=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+))?"
        "(?:\\s+w=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+))?"
        "\\s+collidable_mode=(terrain|small_static|small_moving)"
        "(?:\\s+name=([\\w+-.]+))?$");
    std::smatch match;
    if (Mlib::re::regex_match(line, match, regex)) {
        CreateRigidDisk(renderable_scene()).execute(
            match,
            fpath,
            macro_line_executor,
            local_substitutions,
            rsc);
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
    const std::function<FPath(const std::string&)>& fpath,
    const MacroLineExecutor& macro_line_executor,
    SubstitutionMap* local_substitutions,
    RegexSubstitutionCache& rsc)
{
    std::shared_ptr<RigidBodyVehicle> rb = rigid_disk(
        physics_engine.rigid_bodies_,
        safe_stof(match[4].str()),
        safe_stof(match[5].str()),
        FixedArray<float, 3>{
            match[6].str().empty() ? 0.f : safe_stof(match[6].str()),
            match[7].str().empty() ? 0.f : safe_stof(match[7].str()),
            match[8].str().empty() ? 0.f : safe_stof(match[8].str())},
        FixedArray<float, 3>{
            match[9].str().empty() ? 0.f : safe_stof(match[9].str()),
            match[10].str().empty() ? 0.f : safe_stof(match[10].str()),
            match[11].str().empty() ? 0.f : safe_stof(match[11].str())},
        FixedArray<float, 3>{
            match[12].str().empty() ? 0.f : safe_stof(match[12].str()) * float(M_PI / 180),
            match[13].str().empty() ? 0.f : safe_stof(match[13].str()) * float(M_PI / 180),
            match[14].str().empty() ? 0.f : safe_stof(match[14].str()) * float(M_PI / 180)},
        scene_node_resources.get_geographic_mapping("world"),
        match[16].str());
    std::list<std::shared_ptr<ColoredVertexArray>> hitbox = scene_node_resources.get_animated_arrays(match[2].str())->cvas;
    std::list<std::shared_ptr<ColoredVertexArray>> tirelines;
    if (!match[3].str().empty()) {
        tirelines = scene_node_resources.get_animated_arrays(match[3].str())->cvas;
    }
    CollidableMode collidable_mode = collidable_mode_from_string(match[15].str());
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
