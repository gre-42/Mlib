#include "For_Each_Winner.hpp"
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
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
DECLARE_ARGUMENT(content);
}

ForEachWinner::ForEachWinner(PhysicsScene& physics_scene)
    : LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void ForEachWinner::execute(const LoadSceneJsonUserFunctionArgs& args) {
    if (args.local_json_macro_arguments == nullptr) {
        throw std::runtime_error("Cannot define winner conditionals without local substitutions");
    }
    for (size_t rank = args.arguments.at<size_t>(KnownArgs::begin_rank); rank < args.arguments.at<size_t>(KnownArgs::end_rank); ++rank) {
        auto lapTimeEvent = players.get_winner_track_filename(rank);
        if (!lapTimeEvent.has_value()) {
            continue;
        }
        const auto& lte = *lapTimeEvent;
        if (lte.event.vehicle_colors.empty()) {
            throw std::runtime_error("Could not find a single vehicle color");
        }
        nlohmann::json let{
            {"winner_rank", rank},
            {"winner_suffix", std::to_string(rank)},
            {"winner_vehicle", lte.event.vehicle},
            {"winner_color", lte.event.vehicle_colors[0]}
        };
        args.macro_line_executor.inserted_block_arguments(std::move(let))(args.arguments.at(KnownArgs::content), nullptr);
    }
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "for_each_winner",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                ForEachWinner{args.physics_scene()}.execute(args);
            });
    }
} obj;

}
