#include "Set_Trailer_Hitch_Positions.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Components/Rigid_Body_Vehicle.hpp>
#include <Mlib/Macro_Executor/Asset_Group_Replacement_Parameters.hpp>
#include <Mlib/Macro_Executor/Asset_References.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Replacement_Parameter.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(rigid_body);
DECLARE_ARGUMENT(asset_id);
}

const std::string SetTrailerHitchPositions::key = "set_trailer_hitch_positions";

LoadSceneJsonUserFunction SetTrailerHitchPositions::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    SetTrailerHitchPositions(args.renderable_scene()).execute(args);
};

SetTrailerHitchPositions::SetTrailerHitchPositions(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void SetTrailerHitchPositions::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto& rb = get_rigid_body_vehicle(scene.get_node(args.arguments.at<std::string>(KnownArgs::rigid_body), DP_LOC));
    auto trailer_asset_id = args.arguments.at<std::string>(KnownArgs::asset_id);
    const auto& vars = args
        .asset_references["vehicles"]
        .at(trailer_asset_id)
        .rp;
    if (auto pos = vars.database.at("trailer_hitch_position_female"); pos.type() != nlohmann::detail::value_t::null) {
        rb.trailer_hitches_.set_position_female(pos.get<UFixedArray<float, 3>>());
    }
    if (auto pos = vars.database.at("trailer_hitch_position_male"); pos.type() != nlohmann::detail::value_t::null) {
        rb.trailer_hitches_.set_position_male(pos.get<UFixedArray<float, 3>>());
    }
}
