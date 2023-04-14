#include "Create_Plane_Controller_Key_Binding.hpp"
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Key_Bindings/Plane_Controller_Key_Binding.hpp>
#include <Mlib/Scene/Load_Scene_User_Function_Args.hpp>
#include <Mlib/Scene/Render_Logics/Key_Bindings.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Strings/String.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(ID);
DECLARE_OPTION(ROLE);
DECLARE_OPTION(PLAYER);
DECLARE_OPTION(NODE);

DECLARE_OPTION(TURBINE_POWER);
DECLARE_OPTION(BRAKE);
DECLARE_OPTION(PITCH);
DECLARE_OPTION(YAW);
DECLARE_OPTION(ROLL);

const std::string CreatePlaneControllerKeyBinding::key = "plane_controller_key_binding";

LoadSceneUserFunction CreatePlaneControllerKeyBinding::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^id=([\\w+-.]+)"
        "\\s+role=([\\w+-.]+)"
        "(?:\\s+player=([\\w+-.]+))?"
        "\\s+node=([\\w+-.]+)"

        "(?:\\s+turbine_power=([\\w+-.]+))?"
        "(?:\\s+brake=([\\w+-.]+))?"
        "(?:\\s+pitch=([ \\w+-.]+))?"
        "(?:\\s+yaw=([ \\w+-.]+))?"
        "(?:\\s+roll=([ \\w+-.]+))?$");
    Mlib::re::smatch match;
    if (!Mlib::re::regex_match(args.line, match, regex)) {
        THROW_OR_ABORT("Could not parse user function arguments");
    }
    CreatePlaneControllerKeyBinding(args.renderable_scene()).execute(match, args);
};

CreatePlaneControllerKeyBinding::CreatePlaneControllerKeyBinding(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreatePlaneControllerKeyBinding::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto& node = scene.get_node(match[NODE].str());
    auto& kb = key_bindings.add_plane_controller_key_binding(PlaneControllerKeyBinding{
        .id = match[ID].str(),
        .role = match[ROLE].str(),
        .node = &node,
        .turbine_power = match[TURBINE_POWER].matched
            ? safe_stof(match[TURBINE_POWER].str()) * W
            : std::optional<float>(),
        .brake = match[BRAKE].matched
            ? safe_stof(match[BRAKE].str()) * degrees
            : std::optional<float>(),
        .pitch = match[PITCH].matched
            ? safe_stof(match[PITCH].str()) * degrees
            : std::optional<float>(),
        .yaw = match[YAW].matched
            ? safe_stof(match[YAW].str()) * degrees
            : std::optional<float>(),
        .roll = match[ROLL].matched
            ? safe_stof(match[ROLL].str()) * degrees
            : std::optional<float>(),});
    if (match[PLAYER].matched) {
        players.get_player(match[PLAYER].str())
        .append_delete_externals(
            &node,
            [&kbs=key_bindings, &kb](){
                kbs.delete_plane_controller_key_binding(kb);
            }
        );
    }
}
