#include "Define_Winner_Conditionals.hpp"
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/Physics/Containers/Race_History.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Strings/String_View_To_Number.hpp>
#include <stdexcept>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(begin_rank);
DECLARE_ARGUMENT(end_rank);
}

DefineWinnerConditionals::DefineWinnerConditionals(PhysicsScene& physics_scene)
    : LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void DefineWinnerConditionals::execute(const LoadSceneJsonUserFunctionArgs& args) {
    if (args.local_json_macro_arguments == nullptr) {
        throw std::runtime_error("Cannot define winner conditionals without local substitutions");
    }
    for (size_t rank = args.arguments.at<size_t>(KnownArgs::begin_rank); rank < args.arguments.at<size_t>(KnownArgs::end_rank); ++rank) {
        auto lapTimeEvent = players.get_winner_track_filename(rank);
        if (!lapTimeEvent.has_value()) {
            args.local_json_macro_arguments->merge(JsonMacroArguments(nlohmann::json{
            {
                "IF_WINNER_RANK" + std::to_string(rank) + "_EXISTS",
                false
            },
            {
                "WINNER" + std::to_string(rank) + "_VEHICLE",
                nlohmann::json()
            },
            {
                "WINNER" + std::to_string(rank) + "_COLOR",
                nlohmann::json()
            }}));
        } else {
            const auto& lte = *lapTimeEvent;
            if (lte.event.vehicle_colors.empty()) {
                throw std::runtime_error("Could not find a single vehicle color");
            }
            args.local_json_macro_arguments->merge(JsonMacroArguments(nlohmann::json{
                {
                    "IF_WINNER_RANK" + std::to_string(rank) + "_EXISTS",
                    true
                },
                {
                    "WINNER" + std::to_string(rank) + "_VEHICLE",
                    lte.event.vehicle
                },
                {
                    "WINNER" + std::to_string(rank) + "_COLOR",
                    lte.event.vehicle_colors[0]
                }}));
        }
    }
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "define_winner_conditionals",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                DefineWinnerConditionals{args.physics_scene()}.execute(args);
            });
    }
} obj;

}
