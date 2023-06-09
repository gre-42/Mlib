#include "Json_Expression.hpp"
#include <Mlib/Json/Json_View.hpp>
#include <Mlib/Macro_Executor/Asset_Group_Replacement_Parameters.hpp>
#include <Mlib/Macro_Executor/Asset_References.hpp>
#include <Mlib/Macro_Executor/Asset_References.hpp>
#include <Mlib/Macro_Executor/Replacement_Parameter.hpp>
#include <Mlib/Regex/Match_Counter.hpp>
#include <Mlib/Regex/Misc.hpp>
#include <Mlib/Strings/String.hpp>
#include <type_traits>

namespace DbQueryGroups {
BEGIN_MATCH_COUNTER;
DECLARE_MATCH_COUNTER(group);
DECLARE_MATCH_COUNTER(asset_id);
DECLARE_MATCH_COUNTER(value);
};

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

static nlohmann::json at(
    const std::string& s,
    const nlohmann::json& globals,
    const nlohmann::json& locals)
{
    auto it = globals.find(s);
    if (it == globals.end()) {
        it = locals.find(s);
        if (it == locals.end()) {
            THROW_OR_ABORT("Could not find variable with name \"" + s + '"');
        }
    }
    return *it;
}

static nlohmann::json eval_recursion(
    const std::string& expression,
    const JsonView& globals,
    const JsonView& locals,
    const AssetReferences& asset_references,
    size_t recursion)
{
    if (expression.empty()) {
        return "";
    }
    auto subst_ = [&globals, locals](const std::string& s) {
        auto v = at(s, globals.json(), locals.json());
        if (v.type() != nlohmann::detail::value_t::string) {
            std::stringstream sstr;
            sstr << "Variable \"" << s << "\" is not of type string. Value: \"" << v << '"';
            THROW_OR_ABORT(sstr.str());
        }
        return v.get<std::string>();
    };
    auto subst = [&subst_](const std::string& s){return Mlib::substitute_dollar(s, subst_);};
    if (recursion == 0) {
        if (std::isalpha(expression[0]) ||
            std::isdigit(expression[0]) ||
            (expression[0] == '.') ||
            (expression[0] == '#') ||
            (expression[0] == '-') ||
            (expression[0] == '$') ||
            (expression[0] == '_') ||
            (expression[0] == '^') ||
            (expression[0] == '\\'))
        {
            return subst(expression);
        }
    }
    {
        static const DECLARE_REGEX(comparison_re, "^(\\S+) (==|!=|in|not in) (.*)$");
        Mlib::re::smatch match;
        if (Mlib::re::regex_match(expression, match, comparison_re)) {
            std::string left = match[1].str();
            std::string op = match[2].str();
            std::string right = match[3].str();
            if (op == "==") {
                return eval_recursion(left, globals, locals, asset_references, recursion + 1) ==
                       eval_recursion(right, globals, locals, asset_references, recursion + 1);
            }
            if (op == "!=") {
                return eval_recursion(left, globals, locals, asset_references, recursion + 1) !=
                       eval_recursion(right, globals, locals, asset_references, recursion + 1);
            }
            if (op == "in") {
                auto elems = eval_recursion(right, globals, locals, asset_references, recursion + 1).get<std::set<std::string>>();
                return elems.contains(eval_recursion(left, globals, locals, asset_references, recursion + 1).get<std::string>());
            }
            if (op == "not in") {
                auto elems = eval_recursion(right, globals, locals, asset_references, recursion + 1).get<std::set<std::string>>();
                return !elems.contains(eval_recursion(left, globals, locals, asset_references, recursion + 1).get<std::string>());
            }
            THROW_OR_ABORT("Unknown operator: \"" + op);
        }
    }
    {
        static const DECLARE_REGEX(set_re, "^\\{(.*)\\}$");
        static const DECLARE_REGEX(comma_re, ", ");
        Mlib::re::smatch match;
        if (Mlib::re::regex_match(expression, match, set_re)) {
            std::set<std::string> result;
            for (const auto& element : string_to_list(match[1].str(), comma_re)) {
                if (!result.insert(eval_recursion(element, globals, locals, asset_references, recursion + 1).get<std::string>()).second) {
                    THROW_OR_ABORT("Duplicate element: \"" + element + '"');
                }
            }
            return result;
        }
    }
    {
        static const DECLARE_REGEX(string_re, "^'(.*)'$");
        Mlib::re::smatch match;
        if (Mlib::re::regex_match(expression, match, string_re)) {
            return match[1].str();
        }
    }
    if ((expression[0] == '%') || (expression[0] == '!')) {
        nlohmann::json var;
        if ((expression.length() > 1) && (expression[1] == '%')) {
            static const DECLARE_REGEX(query_re, "^..([^/]+)/([^/]+)/([^/]+)$");
            Mlib::re::smatch match;
            if (!Mlib::re::regex_match(expression, match, query_re)) {
                THROW_OR_ABORT("Could not parse asset-path: \"" + expression + '"');
            }
            var = asset_references
                .get_replacement_parameters(subst(match[DbQueryGroups::group].str()))
                .at(subst(match[DbQueryGroups::asset_id].str()))
                .globals
                .at(match[DbQueryGroups::value].str());
        } else {
            var = at(expression.substr(1), globals.json(), locals.json());
        }
        if (expression[0] == '!') {
            if (var.type() != nlohmann::detail::value_t::boolean) {
                THROW_OR_ABORT("Variable is not of type bool: \"" + expression + '"');
            }
            return !var.get<bool>();
        } else {
            return var;
        }
    }
    THROW_OR_ABORT("Could not interpret \"" + expression + '"');
}

nlohmann::json Mlib::eval(
    const std::string& expression,
    const JsonView& globals,
    const JsonView& locals,
    const AssetReferences& asset_references)
{
    return eval_recursion(expression, globals, locals, asset_references, 0);
}

nlohmann::json Mlib::eval(
    const std::string& expression,
    const JsonView& variables)
{
    AssetReferences asset_references;
    return eval(
        expression,
        variables,
        JsonView{nlohmann::json(nlohmann::json::value_t::object)},
        asset_references);
}

template <>
bool Mlib::eval<bool>(
    const std::string& expression,
    const JsonView& globals,
    const JsonView& locals,
    const AssetReferences& asset_references)
{
    auto result = eval(expression, globals, locals, asset_references);
    if (result.type() != nlohmann::detail::value_t::boolean) {
        THROW_OR_ABORT("Expression is not of type bool: \"" + expression + '"');
    }
    return result;
}

template <>
bool Mlib::eval<bool>(
    const std::string& expression,
    const JsonView& variables)
{
    AssetReferences asset_references;
    return eval<bool>(
        expression,
        variables,
        JsonView{nlohmann::json(nlohmann::json::value_t::object)},
        asset_references);
}
