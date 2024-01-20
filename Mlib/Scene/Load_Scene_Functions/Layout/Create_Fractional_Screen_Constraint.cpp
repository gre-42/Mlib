#include "Create_Fractional_Screen_Constraint.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Layout/Concrete_Layout_Pixels.hpp>
#include <Mlib/Layout/Layout_Constraints.hpp>
#include <Mlib/Layout/Screen_Units.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Strings/To_Number.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(value);
DECLARE_ARGUMENT(anchor_a);
DECLARE_ARGUMENT(anchor_b);
}

const std::string CreateFractionalScreenConstraint::key = "fractional_screen_constraint";

LoadSceneJsonUserFunction CreateFractionalScreenConstraint::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    args.layout_constraints.set_pixels(
        args.arguments.at<std::string>(KnownArgs::name),
        std::make_unique<FractionalConstraint>(
            args.arguments.at<float>(KnownArgs::value),
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::anchor_a)),
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::anchor_b))));
};
