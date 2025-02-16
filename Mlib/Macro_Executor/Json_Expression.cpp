#include "Json_Expression.hpp"
#include <Mlib/Json/Json_View.hpp>
#include <Mlib/Macro_Executor/Asset_Group_Replacement_Parameters.hpp>
#include <Mlib/Macro_Executor/Asset_References.hpp>
#include <Mlib/Macro_Executor/Replacement_Parameter.hpp>
#include <Mlib/Regex/Match_Counter.hpp>
#include <Mlib/Regex/Misc.hpp>
#include <Mlib/Regex/Template_Regex.hpp>
#include <Mlib/Strings/String.hpp>
#include <Mlib/Strings/String_View_To_Number.hpp>

namespace DbQueryGroups {
BEGIN_MATCH_COUNTER;
DECLARE_MATCH_COUNTER(group);
DECLARE_MATCH_COUNTER(asset_id);
DECLARE_MATCH_COUNTER(value);
};

namespace DictQueryGroups {
BEGIN_MATCH_COUNTER;
DECLARE_MATCH_COUNTER(dict);
DECLARE_MATCH_COUNTER(key);
};

using namespace Mlib;
using namespace Mlib::TemplateRegex;

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

static const nlohmann::json& at(
    const std::string_view& s,
    const nlohmann::json& globals,
    const nlohmann::json& locals)
{
    auto it = globals.find(s);
    if (it == globals.end()) {
        it = locals.find(s);
        if (it == locals.end()) {
            THROW_OR_ABORT("Could not find variable with name \"" + std::string{ s } + '"');
        }
    }
    return *it;
}

static const auto sl = chr('/');
static const auto NSL = group(plus(CharPredicate([](char c){ return c != '/'; })));
static const auto W = group(plus(CharPredicate(is_word)));
static const auto NC = group(plus(CharPredicate([](char c){ return c != ','; })));

static nlohmann::json eval_recursion(
    std::string_view expression,
    const JsonView& globals,
    const JsonView& locals,
    const AssetReferences& asset_references,
    size_t recursion)
{
    if (recursion > 100) {
        THROW_OR_ABORT("Detected possibly infinite recursion");
    }
    if (expression.empty()) {
        return "";
    }
    std::function<std::string(const std::string_view&)> subst;
    auto subst_ = [&globals, &locals, &asset_references, &subst](const std::string_view& s) {
        if (s.empty()) {
            THROW_OR_ABORT("Received empty substitution variable");
        }
        if (s[0] == '$') {
            static const auto query_re = seq(adot, NSL, sl, NSL, sl, W, eof);
            SMatch<4> match;
            if (!regex_match(s, match, query_re)) {
                THROW_OR_ABORT("Could not parse asset path: \"" + std::string{ s } + '"');
            }
            return asset_references[subst(match[DbQueryGroups::group].str())]
                .at(subst(match[DbQueryGroups::asset_id].str()))
                .rp
                .database
                .at<std::string>(match[DbQueryGroups::value].str());
        }
        const auto& v = at(s, globals.json(), locals.json());
        if (v.type() != nlohmann::detail::value_t::string) {
            std::stringstream sstr;
            sstr << "Variable \"" << s << "\" is not of type string. Value: \"" << v << '"';
            THROW_OR_ABORT(sstr.str());
        }
        return v.get<std::string>();
    };
    subst = [&subst_](const std::string_view& s){return Mlib::substitute_dollar(s, subst_);};
    if (recursion == 0) {
        if (std::isalpha(expression[0]) ||
            (expression[0] == '.')  ||
            (expression[0] == '#')  ||
            (expression[0] == '-')  ||
            (expression[0] == '$')  ||
            (expression[0] == '_')  ||
            (expression[0] == '^')  ||
            (expression[0] == '\\') ||
            (expression[0] == '/'))
        {
            return subst(expression);
        }
    }
    {
        // static const DECLARE_REGEX(comparison_re, "^(\\S+) (==|!=|in|not in) (.+)$");
        static const auto comparison_re = seq(
            group(plus(no_space)),
            chr(' '),
            group(par(str("=="), str("!="), str("in"), str("not in"))),
            chr(' '),
            group(plus(adot)),
            eof);
        if (SMatch<4> match; regex_match(expression, match, comparison_re)) {
            std::string_view left = match[1].str();
            std::string_view op = match[2].str();
            std::string_view right = match[3].str();
            if (op == "==") {
                return eval_recursion(left, globals, locals, asset_references, recursion + 1) ==
                       eval_recursion(right, globals, locals, asset_references, recursion + 1);
            }
            if (op == "!=") {
                return eval_recursion(left, globals, locals, asset_references, recursion + 1) !=
                       eval_recursion(right, globals, locals, asset_references, recursion + 1);
            }
            if (op == "in") {
                auto elems = eval_recursion(right, globals, locals, asset_references, recursion + 1).get<std::set<nlohmann::json>>();
                return elems.contains(eval_recursion(left, globals, locals, asset_references, recursion + 1));
            }
            if (op == "not in") {
                auto elems = eval_recursion(right, globals, locals, asset_references, recursion + 1).get<std::set<nlohmann::json>>();
                return !elems.contains(eval_recursion(left, globals, locals, asset_references, recursion + 1));
            }
            THROW_OR_ABORT("Unknown operator: \"" + std::string{ op } + '"');
        }
    }
    if ((expression.size() >= 2) && (expression[0] == '{') && (expression[expression.size() - 1] == '}')) {
        // static const DECLARE_REGEX(set_re, "^\\{(.*)\\}$");
        // static const DECLARE_REGEX(comma_re, ", ");
        static const auto comma_re = par(str(", "), NC);
        std::set<nlohmann::json> result;
        find_all_templated(expression.substr(1, expression.size() - 2), comma_re, [&](const SMatch<2>& match2b) {
            if (match2b[1].matched) {
                if (!result.insert(eval_recursion(match2b[1].str(), globals, locals, asset_references, recursion + 1)).second) {
                    THROW_OR_ABORT("Duplicate element: \"" + std::string{ match2b[1].str() } + '"');
                }
            }
            });
        return result;
    }
    {
        // static const DECLARE_REGEX(string_re, "^'(.*)'$");
        // // Disabled this code because "group(star(adot))" is greedy and eats away the trailing "'"
        // static const auto string_re = seq(chr('\''), group(star(adot)), chr('\''), eof);
        // if (SMatch<2> match; regex_match(expression, match, string_re)) {
        //     return match[1].str();
        // }
        if ((expression.size() >= 2) && (expression[0] == '\'') && (expression[expression.size() - 1] == '\'')) {
            return expression.substr(1, expression.size() - 2);
        }
    }
    {
        // static const DECLARE_REGEX(int_re, "^(\\d+)$");
        static const auto int_re = seq(plus(digit), eof);
        if (SMatch<1> match; regex_match(expression, match, int_re)) {
            return safe_stoi(match[0].str());
        }
    }
    {
        // static const DECLARE_REGEX(float_re, "^(\\d+\\.\\d+f)$");
        static const auto float_re = seq(plus(digit), chr('.'), plus(digit), chr('f'), eof);
        if (SMatch<1> match; regex_match(expression, match, float_re)) {
            return safe_stof(match[0].str());
        }
    }
    {
        // static const DECLARE_REGEX(double_re, "^(\\d+\\.\\d+)$");
        static const auto double_re = seq(plus(digit), chr('.'), plus(digit), eof);
        if (SMatch<1> match; regex_match(expression, match, double_re)) {
            return safe_stod(match[0].str());
        }
    }
    if ((expression[0] == '%') || (expression[0] == '!')) {
        if (expression == "%null") {
            return nlohmann::json();
        }
        nlohmann::json var;
        if ((expression.length() > 1) && (expression[1] == '%')) {
            // static const DECLARE_REGEX(query_re, "^..([^/]+)/([^/]+)/(\\w+)$");
            static const auto query_re = seq(adot, adot, NSL, sl, NSL, sl, W, eof);
            SMatch<4> match;
            if (!regex_match(expression, match, query_re)) {
                THROW_OR_ABORT("Could not parse asset path: \"" + std::string{ expression } + '"');
            }
            var = asset_references[subst(match[DbQueryGroups::group].str())]
                .at(subst(match[DbQueryGroups::asset_id].str()))
                .rp
                .database
                .at(match[DbQueryGroups::value].str());
        } else if ((expression.length() > 1) && (expression[1] == '/')) {
            // static const DECLARE_REGEX(query_re, "^..([^/]+)/([^/]+)$");
            static const auto query_re = seq(adot, adot, NSL, sl, NSL, eof);
            SMatch<3> match;
            if (!regex_match(expression, match, query_re)) {
                THROW_OR_ABORT("Could not parse asset path: \"" + std::string{ expression } + '"');
            }
            auto dict_name = subst(match[DictQueryGroups::dict].str());
            const auto& dict = at(dict_name, globals.json(), locals.json());
            if (dict.type() != nlohmann::detail::value_t::object) {
                THROW_OR_ABORT("Variable \"" + dict_name + "\" is not a dictionary");
            }
            auto key_name = subst(match[DictQueryGroups::key].str());
            var = JsonView{ dict }.at(key_name);
        } else {
            var = at(expression.substr(1), globals.json(), locals.json());
        }
        if (expression[0] == '!') {
            if (var.type() != nlohmann::detail::value_t::boolean) {
                THROW_OR_ABORT("Variable is not of type bool: \"" + std::string{ expression } + '"');
            }
            return !var.get<bool>();
        } else {
            return var;
        }
    }
    THROW_OR_ABORT("Could not interpret \"" + std::string{ expression } + '"');
}

nlohmann::json Mlib::eval(
    std::string_view expression,
    const JsonView& globals,
    const JsonView& locals,
    const AssetReferences& asset_references)
{
    return eval_recursion(expression, globals, locals, asset_references, 0);
}

nlohmann::json Mlib::eval(
    std::string_view expression,
    const JsonView& variables,
    const AssetReferences& asset_references)
{
    return eval(
        expression,
        variables,
        JsonView{ nlohmann::json::object() },
        asset_references);
}

nlohmann::json Mlib::eval(
    std::string_view expression,
    const JsonView& variables)
{
    return eval(
        expression,
        variables,
        AssetReferences{});
}

// bool

template <>
bool Mlib::eval<bool>(
    std::string_view expression,
    const JsonView& globals,
    const JsonView& locals,
    const AssetReferences& asset_references)
{
    auto result = eval(expression, globals, locals, asset_references);
    if (result.type() != nlohmann::detail::value_t::boolean) {
        THROW_OR_ABORT("Expression is not of type bool: \"" + std::string{ expression } + '"');
    }
    return result;
}

template <>
bool Mlib::eval<bool>(
    std::string_view expression,
    const JsonView& variables,
    const AssetReferences& asset_references)
{
    return eval<bool>(
        expression,
        variables,
        JsonView{ nlohmann::json::object() },
        asset_references);
}

template <>
bool Mlib::eval<bool>(
    std::string_view expression,
    const JsonView& variables)
{
    return eval<bool>(
        expression,
        variables,
        AssetReferences{});

}

// string

template <>
std::string Mlib::eval<std::string>(
    std::string_view expression,
    const JsonView& globals,
    const JsonView& locals,
    const AssetReferences& asset_references)
{
    auto result = eval(expression, globals, locals, asset_references);
    if (result.type() != nlohmann::detail::value_t::string) {
        THROW_OR_ABORT("Expression is not of type string: \"" + std::string{ expression } + '"');
    }
    return result;
}

template <>
std::string Mlib::eval<std::string>(
    std::string_view expression,
    const JsonView& variables,
    const AssetReferences& asset_references)
{
    return eval<std::string>(
        expression,
        variables,
        JsonView{ nlohmann::json::object() },
        asset_references);
}

template <>
std::string Mlib::eval<std::string>(
    std::string_view expression,
    const JsonView& variables)
{
    return eval<std::string>(
        expression,
        variables,
        AssetReferences{});

}
