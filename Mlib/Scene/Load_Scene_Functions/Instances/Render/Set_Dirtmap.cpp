#include "Set_Dirtmap.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Geometry/Material/Wrap_Mode.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Render/Resource_Managers/Rendering_Resources.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(filename);
DECLARE_ARGUMENT(offset);
DECLARE_ARGUMENT(discreteness);
DECLARE_ARGUMENT(scale);
}

const std::string SetDirtmap::key = "set_dirtmap";

LoadSceneJsonUserFunction SetDirtmap::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    SetDirtmap(args.physics_scene()).execute(args);
};

SetDirtmap::SetDirtmap(PhysicsScene& physics_scene) 
: LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

static const auto dirtmap_name = VariableAndHash<std::string>("dirtmap");

void SetDirtmap::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    rendering_resources.set_alias(dirtmap_name, VariableAndHash{ args.arguments.path_or_variable(KnownArgs::filename).path });
    rendering_resources.set_offset(dirtmap_name, args.arguments.at<float>(KnownArgs::offset));
    rendering_resources.set_discreteness(dirtmap_name, args.arguments.at<float>(KnownArgs::discreteness));
    rendering_resources.set_scale(dirtmap_name, args.arguments.at<float>(KnownArgs::scale));
}
