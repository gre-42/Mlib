#include "Create_Fractional_Screen_Constraint.hpp"
#include <Mlib/Layout/Concrete_Layout_Pixels.hpp>
#include <Mlib/Layout/Layout_Constraints.hpp>
#include <Mlib/Layout/Screen_Units.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Strings/To_Number.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(NAME);
DECLARE_OPTION(VALUE);
DECLARE_OPTION(ANCHOR_A);
DECLARE_OPTION(ANCHOR_B);

const std::string CreateFractionalScreenConstraint::key = "fractional_screen_constraint";

LoadSceneUserFunction CreateFractionalScreenConstraint::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^name=(\\w+)"
        "\\s+value=([\\w+-.]+)"
        "\\s+anchor_a=(\\w+)"
        "\\s+anchor_b=(\\w+)$");
    Mlib::re::smatch match;
    if (!Mlib::re::regex_match(args.line, match, regex)) {
        THROW_OR_ABORT("Could not parse user function arguments");
    }
    execute(match, args);
};

void CreateFractionalScreenConstraint::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    args.layout_constraints.set_pixels(
        match[NAME].str(),
        std::make_unique<FractionalConstraint>(
            safe_stof(match[VALUE].str()),
            args.layout_constraints.get_pixels(match[ANCHOR_A].str()),
            args.layout_constraints.get_pixels(match[ANCHOR_B].str())));
}
