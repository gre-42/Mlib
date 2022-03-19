#include "Create_Gun.hpp"
#include <Mlib/Physics/Advance_Times/Gun.hpp>
#include <Mlib/Physics/Physics_Engine.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/Linker.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

LoadSceneUserFunction CreateGun::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*gun"
        "\\s+node=([\\w+-.]+)"
        "\\s+parent_rigid_body_node=([\\w+-.]+)"
        "\\s+punch_angle_node=([\\w+-.]+)"
        "\\s+cool_down=([\\w+-.]+)"
        "\\s+renderable=([\\w-. \\(\\)/+-]+)"
        "\\s+hitbox=([\\w-. \\(\\)/+-]+)"
        "\\s+mass=([\\w+-.]+)"
        "\\s+velocity=([\\w+-.]+)"
        "\\s+lifetime=([\\w+-.]+)"
        "\\s+damage=([\\w+-.]+)"
        "\\s+size=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+punch_angle=([\\w+-.]+)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        CreateGun(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

CreateGun::CreateGun(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateGun::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    Linker linker{ physics_engine.advance_times_ };
    auto& parent_rb_node = scene.get_node(match[2].str());
    auto rb = dynamic_cast<RigidBodyVehicle*>(parent_rb_node.get_absolute_movable());
    if (rb == nullptr) {
        throw std::runtime_error("Absolute movable is not a rigid body");
    }
    auto& punch_angle_node = scene.get_node(match[3].str());
    std::shared_ptr<Gun> gun = std::make_shared<Gun>(
        scene,                                           // scene
        scene_node_resources,                            // scene_node_resources
        physics_engine.rigid_bodies_,                    // rigid_bodies
        physics_engine.advance_times_,                   // advance_times
        safe_stof(match[4].str()) * s,                   // cool_down
        rb->rbi_,                                        // parent_rigid_body_node
        punch_angle_node,                                // punch_angle_node
        match[5].str(),                                  // bullet-renderable-resource-name
        match[6].str(),                                  // bullet-hitbox-resource-name
        safe_stof(match[7].str()),                       // bullet-mass
        safe_stof(match[8].str()) * meters / s,          // bullet_velocity
        safe_stof(match[9].str()) * s,                   // bullet-lifetime
        safe_stof(match[10].str()),                      // bullet-damage
        FixedArray<float, 3>{                            // bullet-size
            safe_stof(match[11].str()),                  // bullet-size-x
            safe_stof(match[12].str()),                  // bullet-size-y
            safe_stof(match[13].str())},                 // bullet-size-z
        safe_stof(match[14].str()) * float(M_PI / 180),  // punch_angle
        delete_node_mutex);                              // delete_node_mutex
        
    linker.link_absolute_observer(scene.get_node(match[1].str()), gun);
}
