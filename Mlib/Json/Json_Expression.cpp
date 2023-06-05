#include "Json_Expression.hpp"
#include <Mlib/Json/Json_View.hpp>
#include <Mlib/Regex_Select.hpp>
#include <type_traits>

using namespace Mlib;

template <class T>
struct FalseJsonEval {
    static const bool value = false;
};

template <class T>
static T get(
    const std::string& var,
    const JsonView& globals,
    const JsonView& locals)
{
    auto g = globals.try_at<T>(var);
    if (g.has_value()) {
        return g.value();
    } else {
        return locals.at<T>(var);
    }
}

template <class T>
T Mlib::eval(
    const std::string& expression,
    const JsonView& globals,
    const JsonView& locals)
{
    if constexpr (std::is_same_v<T, bool>) {
        static const DECLARE_REGEX(equality_re, "^(\\w+)=='(\\w+)'$");
        static const DECLARE_REGEX(inequality_re, "^(\\w+)!='(\\w+)'$");
        Mlib::re::smatch match;
        if (Mlib::re::regex_match(expression, match, equality_re)) {
            return get<std::string>(match[1].str(), globals, locals) == match[2].str();
        }
        if (Mlib::re::regex_match(expression, match, inequality_re)) {
            return get<std::string>(match[1].str(), globals, locals) != match[2].str();
        }
        return get<bool>(expression, globals, locals);
    } else {
        static_assert(FalseJsonEval<T>::value, "Unsupported type in Mlib::eval");
    }
}

template <class T>
T Mlib::eval(const std::string& expression, const JsonView& variables) {
    return eval<T>(expression, variables, JsonView{{}});
}

template bool Mlib::eval<bool>(const std::string& expression, const JsonView& variables);
