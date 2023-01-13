#include "Create_Constant_Screen_Constraint.hpp"
#include <Mlib/Layout/Concrete_Layout_Constraints.hpp>
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
DECLARE_OPTION(UNITS);

LoadSceneUserFunction CreateConstantScreenConstraint::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*constant_screen_constraint"
        "\\s+name=(\\w+)"
        "\\s+value=([\\w+-.]+)"
        "\\s+units=(\\w+)$");
    Mlib::re::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        execute(match, args);
        return true;
    } else {
        return false;
    }
};

void CreateConstantScreenConstraint::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    args.layout_constraints.insert(
        match[NAME].str(),
        std::make_unique<ConstantConstraint>(
            safe_stof(match[VALUE].str()),
            screen_units_from_string(match[UNITS].str())));
}
