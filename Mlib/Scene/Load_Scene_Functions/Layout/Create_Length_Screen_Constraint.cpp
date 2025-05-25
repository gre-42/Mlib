#include <Mlib/Argument_List.hpp>
#include <Mlib/Layout/Concrete_Layout_Pixels.hpp>
#include <Mlib/Layout/Layout_Constraints.hpp>
#include <Mlib/Layout/Screen_Units.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Strings/To_Number.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(value);
DECLARE_ARGUMENT(units);
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "length_screen_constraint",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                args.arguments.validate(KnownArgs::options);
                args.layout_constraints.set_pixels(
                    args.arguments.at<std::string>(KnownArgs::name),
                    std::make_unique<LengthConstraint>(
                        args.arguments.at<float>(KnownArgs::value),
                        screen_units_from_string(args.arguments.at<std::string>(KnownArgs::units))));
            });
    }
} obj;

}
