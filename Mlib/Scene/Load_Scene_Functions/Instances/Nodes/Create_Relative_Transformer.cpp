#include "Create_Relative_Transformer.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Relative_Transformer.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Linker.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Strings/To_Number.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(node);
DECLARE_ARGUMENT(v);
DECLARE_ARGUMENT(w);
}

const std::string CreateRelativeTransformer::key = "relative_transformer";

LoadSceneJsonUserFunction CreateRelativeTransformer::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    CreateRelativeTransformer(args.renderable_scene()).execute(args);
};

CreateRelativeTransformer::CreateRelativeTransformer(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateRelativeTransformer::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    Linker linker{ physics_engine.advance_times_ };
    auto v = args.arguments.at<FixedArray<float, 3>>(KnownArgs::v, fixed_zeros<float, 3>()) * meters / s;
    auto w = args.arguments.at<FixedArray<float, 3>>(KnownArgs::w, fixed_zeros<float, 3>()) * degrees / s;
    auto rt = std::make_unique<RelativeTransformer>(physics_engine.advance_times_, v, w);
    linker.link_relative_movable(scene.get_node(args.arguments.at<std::string>(KnownArgs::node), DP_LOC), std::move(rt));
}
