#include "Json_Expression.hpp"
#include <Mlib/Json/Json_View.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Strings/String.hpp>
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

template <>
std::string Mlib::eval<std::string>(
    const std::string& expression,
    const JsonView& globals,
    const JsonView& locals)
{
    static const DECLARE_REGEX(string_literal_re, "'(\\w*)'");
    Mlib::re::smatch match;
    if (Mlib::re::regex_match(expression, match, string_literal_re)) {
        return match[1].str();
    }
    return get<std::string>(expression, globals, locals);
}

template <>
std::set<std::string> Mlib::eval<std::set<std::string>>(
    const std::string& expression,
    const JsonView& globals,
    const JsonView& locals)
{
    static const DECLARE_REGEX(in_re, "^\\{(.*)\\}$");
    static const DECLARE_REGEX(comma_re, ", ");
    Mlib::re::smatch match;
    std::set<std::string> result;
    if (!Mlib::re::regex_match(expression, match, in_re)) {
        THROW_OR_ABORT("Could not parse as set: \"" + expression + '"');
    }
    for (const auto& element : string_to_list(match[1].str(), comma_re)) {
        if (!result.insert(eval<std::string>(element, globals, locals)).second) {
            THROW_OR_ABORT("Duplicate element: \"" + element + '"');
        }
    }
    return result;
}

template <>
bool Mlib::eval<bool>(
    const std::string& expression,
    const JsonView& globals,
    const JsonView& locals)
{
    static const DECLARE_REGEX(in_re, "^(\\w+) in (.*)$");
    static const DECLARE_REGEX(not_in_re, "^(\\w+) not in (.*)$");
    Mlib::re::smatch match;
    if (Mlib::re::regex_match(expression, match, in_re)) {
        auto elems = eval<std::set<std::string>>(match[2].str(), globals, locals);
        return elems.contains(eval<std::string>(match[1].str(), globals, locals));
    }
    if (Mlib::re::regex_match(expression, match, not_in_re)) {
        auto elems = eval<std::set<std::string>>(match[2].str(), globals, locals);
        return !elems.contains(eval<std::string>(match[1].str(), globals, locals));
    }
    return get<bool>(expression, globals, locals);
}

template <class T>
T Mlib::eval(const std::string& expression, const JsonView& variables) {
    return eval<T>(expression, variables, JsonView{std::map<std::string, std::string>{}});
}

template bool Mlib::eval<bool>(const std::string& expression, const JsonView& variables);
