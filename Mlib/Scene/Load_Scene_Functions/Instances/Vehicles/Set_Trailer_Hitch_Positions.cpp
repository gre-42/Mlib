#include "Set_Trailer_Hitch_Positions.hpp"
#include <Mlib/Argument_List.hpp>
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
    auto rb = dynamic_cast<RigidBodyVehicle*>(&scene.get_node(args.arguments.at<std::string>(KnownArgs::rigid_body)).get_absolute_movable());
    if (rb == nullptr) {
        THROW_OR_ABORT("Absolute movable is not a rigid body");
    }
    auto trailer_asset_id = args.arguments.at<std::string>(KnownArgs::asset_id);
    const auto& vars = args
        .asset_references
        .get_replacement_parameters("vehicles")
        .at(trailer_asset_id);
    if (auto pos = vars.globals.at("TRAILER_HITCH_POSITION_FEMALE"); pos.type() != nlohmann::detail::value_t::null) {
        rb->trailer_hitches_.set_position_female(pos.get<FixedArray<float, 3>>());
    }
    if (auto pos = vars.globals.at("TRAILER_HITCH_POSITION_MALE"); pos.type() != nlohmann::detail::value_t::null) {
        rb->trailer_hitches_.set_position_male(pos.get<FixedArray<float, 3>>());
    }
}