#include "Set_Preferred_Car_Spawner.hpp"
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Macro_Line_Executor.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Players/Advance_Times/Game_Logic.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Spawn_Point.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(PLAYER);
DECLARE_OPTION(MACRO);
DECLARE_OPTION(PARAMETERS);

LoadSceneUserFunction SetPreferredCarSpawner::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*set_preferred_car_spawner"
        "\\s+player=([\\w+-.]+)"
        "\\s+macro=([\\w.]+)"
        "\\s+parameters=([\\s\\S]*)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        SetPreferredCarSpawner(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

SetPreferredCarSpawner::SetPreferredCarSpawner(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void SetPreferredCarSpawner::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto primary_rendering_context = RenderingContextStack::primary_resource_context();
    auto secondary_rendering_context = RenderingContextStack::resource_context();
    std::string player = match[1].str();
    std::string macro = match[2].str();
    std::string parameters = match[3].str();
    game_logic.spawn.set_preferred_car_spawner(
        players.get_player(player),
        [macro_line_executor = args.macro_line_executor,
         player,
         macro,
         parameters,
         primary_rendering_context,
         secondary_rendering_context,
         &scene = scene,
         &rsc = args.rsc](const SpawnPoint& p){
            RenderingContextGuard rrg0{primary_rendering_context};
            RenderingContextGuard rrg1{secondary_rendering_context};
            auto z = z3_from_3x3(tait_bryan_angles_2_matrix(p.rotation));
            std::stringstream sstr;
            sstr <<
                "macro_playback " <<
                macro <<
                " HUMAN_NODE_X:" << p.position(0) <<
                " HUMAN_NODE_Y:" << p.position(1) <<
                " HUMAN_NODE_Z:" << p.position(2) <<
                " HUMAN_NODE_ANGLE_Y:" << std::atan2(z(0), z(2)) / degrees <<
                " CAR_NODE_X:" << p.position(0) <<
                " CAR_NODE_Y:" << p.position(1) <<
                " CAR_NODE_Z:" << p.position(2) <<
                " CAR_NODE_ANGLE_X:" << p.rotation(0) / degrees <<
                " CAR_NODE_ANGLE_Y:" << p.rotation(1) / degrees <<
                " CAR_NODE_ANGLE_Z:" << p.rotation(2) / degrees <<
                " " << parameters <<
                " SUFFIX:_" << player + '_' + std::to_string(scene.get_uuid()) <<
                " IF_WITH_GRAPHICS:" <<
                " IF_WITH_PHYSICS:" <<
                " IF_RACING:#" <<
                " IF_RALLY:" <<
                " PLAYER_NAME:" << player;
            macro_line_executor(sstr.str(), nullptr, rsc);
        }
    );
}
