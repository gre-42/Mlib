#include "Set_Dirtmap.hpp"
#include <Mlib/Geometry/Material/Wrap_Mode.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/OpenGL/Render_Logic.hpp>
#include <Mlib/OpenGL/Resource_Managers/Rendering_Resources.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>

using namespace Mlib;

static const auto dirtmap_name = VariableAndHash<std::string>("dirtmap");

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(filename);
DECLARE_ARGUMENT(offset);
DECLARE_ARGUMENT(discreteness);
DECLARE_ARGUMENT(scale);
}

SetDirtmap::SetDirtmap(PhysicsScene& physics_scene)
    : LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void SetDirtmap::execute(const LoadSceneJsonUserFunctionArgs& args) {
    args.arguments.validate(KnownArgs::options);
    rendering_resources.set_alias(dirtmap_name, args.arguments.path_or_variable(KnownArgs::filename));
    rendering_resources.set_offset(dirtmap_name, args.arguments.at<float>(KnownArgs::offset));
    rendering_resources.set_discreteness(dirtmap_name, args.arguments.at<float>(KnownArgs::discreteness));
    rendering_resources.set_scale(dirtmap_name, args.arguments.at<float>(KnownArgs::scale));
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "set_dirtmap",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                SetDirtmap{args.physics_scene()}.execute(args);
            });
    }
} obj;

}
