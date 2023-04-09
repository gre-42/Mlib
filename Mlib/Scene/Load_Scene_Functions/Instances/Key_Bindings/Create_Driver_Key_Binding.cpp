#include "Create_Driver_Key_Binding.hpp"
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Key_Bindings/Player_Key_Binding.hpp>
#include <Mlib/Scene/Render_Logics/Key_Bindings.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Strings/String.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(ID);
DECLARE_OPTION(ROLE);

DECLARE_OPTION(NODE);

DECLARE_OPTION(SELECT_NEXT_OPPONENT);
DECLARE_OPTION(SELECT_NEXT_VEHICLE);

const std::string CreateDriverKeyBinding::key = "player_key_binding";

LoadSceneUserFunction CreateDriverKeyBinding::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^id=([\\w+-.]+)"
        "\\s+role=([\\w+-.]+)"

        "\\s+node=([\\w+-.]+)"

        "(\\s+select_next_opponent)?"
        "(\\s+select_next_vehicle)?$");
    Mlib::re::smatch match;
    if (!Mlib::re::regex_match(args.line, match, regex)) {
        THROW_OR_ABORT("Could not parse user function arguments");
    }
    CreateDriverKeyBinding(args.renderable_scene()).execute(match, args);
};

CreateDriverKeyBinding::CreateDriverKeyBinding(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateDriverKeyBinding::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto& node = scene.get_node(match[NODE].str());
    auto& kb = key_bindings.add_player_key_binding(PlayerKeyBinding{
        .id = match[ID].str(),
        .role = match[ROLE].str(),
        .node = &node,
        .select_next_opponent = match[SELECT_NEXT_OPPONENT].matched,
        .select_next_vehicle = match[SELECT_NEXT_VEHICLE].matched});
    auto rb = dynamic_cast<RigidBodyVehicle*>(&node.get_absolute_movable());
    if (rb == nullptr) {
        THROW_OR_ABORT("Absolute movable is not a rigid body");
    }
    if (rb->driver_ == nullptr) {
        THROW_OR_ABORT("Rigid body has no driver");
    }
    auto player = dynamic_cast<Player*>(rb->driver_);
    if (player == nullptr) {
        THROW_OR_ABORT("Driver is not player");
    }
    player->append_delete_externals(
        &node,
        [&kbs=key_bindings, &kb](){
            kbs.delete_player_key_binding(kb);
        }
    );
}
