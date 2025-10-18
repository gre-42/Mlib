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
DECLARE_MATCH_COUNTER(key);
}

namespace DictQueryGroups {
BEGIN_MATCH_COUNTER;
DECLARE_MATCH_COUNTER(dict);
DECLARE_MATCH_COUNTER(key);
}

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
    const nlohmann::json& locals,
    const nlohmann::json& block)
{
    auto git = globals.find(s);
    auto lit = locals.find(s);
    auto bit = block.find(s);
    auto nfound = int(git != globals.end()) + int(lit != locals.end()) + int(bit != block.end());
    if (nfound == 0) {
        THROW_OR_ABORT("Could not find variable: \"" + std::string{ s } + '"');
    }
    if (nfound > 1) {
        THROW_OR_ABORT("Found variable in multiple dictionaries (globals, locals, block): \"" + std::string{ s } + '"');
    }
    if (git != globals.end()) {
        return *git;
    }
    if (lit != locals.end()) {
        return *lit;
    }
    if (bit != block.end()) {
        return *bit;
    }
    verbose_abort("Get variable internal error");
}

static const auto sl = chr('/');
static const auto NSL = group(plus(CharPredicate([](char c){ return c != '/'; })));
static const auto W = group(plus(CharPredicate(is_word)));
static const auto NC = group(plus(CharPredicate([](char c){ return c != ','; })));

static nlohmann::json eval_recursion(
    std::string_view expression,
    const JsonView& globals,
    const JsonView& locals,
    const JsonView& block,
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
    auto subst_ = [&globals, &locals, &block, &asset_references, &subst](const std::string_view& s) {
        if (s.empty()) {
            THROW_OR_ABORT("Received empty substitution variable");
        }
        if (s[0] == '$') {
            static const auto query_re = seq(adot, NSL, sl, NSL, sl, NSL, opt(seq(sl, NSL)), eof);
            SMatch<5> match;
            if (!regex_match(s, match, query_re)) {
                THROW_OR_ABORT("Could not parse asset path: \"" + std::string{ s } + '"');
            }
            const auto& db = asset_references[subst(match[DbQueryGroups::group].str())]
                .at(subst(match[DbQueryGroups::asset_id].str()))
                .rp
                .database;
            if (match[DbQueryGroups::key].matched()) {
                auto res = db.at(match[DbQueryGroups::value].str());
                if (res.type() != nlohmann::detail::value_t::object) {
                    THROW_OR_ABORT("Database value is not of type object: \"" + std::string{ s } + '"');
                }
                auto key = subst(match[DbQueryGroups::key].str());
                auto it = res.find(key);
                if (it == res.end()) {
                    THROW_OR_ABORT("Could not find database key \"" + std::string{ key } + "\": \"" + std::string{ s } + '"');
                }
                if (it->type() != nlohmann::detail::value_t::string) {
                    THROW_OR_ABORT("Database value is not of type string: \"" + std::string{ s } + '"');
                }
                return it->get<std::string>();
            } else {
                return db.at<std::string>(match[DbQueryGroups::value].str());
            }
        } else if (s[0] == '/') {
            // static const DECLARE_REGEX(query_re, "^.([^/]+)/([^/]+)$");
            static const auto query_re = seq(adot, NSL, sl, NSL, eof);
            SMatch<3> match;
            if (!regex_match(s, match, query_re)) {
                THROW_OR_ABORT("Could not parse asset path: \"" + std::string{ s } + '"');
            }
            auto dict_name = subst(match[DictQueryGroups::dict].str());
            const auto& dict = at(dict_name, globals.json(), locals.json(), block.json());
            if (dict.type() != nlohmann::detail::value_t::object) {
                THROW_OR_ABORT("Variable \"" + dict_name + "\" is not a dictionary");
            }
            auto key_name = subst(match[DictQueryGroups::key].str());
            return JsonView{ dict }.at<std::string>(key_name);
        } else {
            const auto& v = at(s, globals.json(), locals.json(), block.json());
            if (v.type() != nlohmann::detail::value_t::string) {
                std::stringstream sstr;
                sstr << "Variable \"" << s << "\" is not of type string. Value: \"" << v << '"';
                THROW_OR_ABORT(sstr.str());
            }
            return v.get<std::string>();
        }
    };
    {
        // static const DECLARE_REGEX(comparison_re, "^\((\\S+) (==|!=|in|not in) (.+)\)$");
        static const auto comparison_re = seq(
            chr('('),
            group(plus(no_space)),
            chr(' '),
            group(par(str("=="), str("!="), str("<="), str(">="), str("in"), str("not in"), chr('-'))),
            chr(' '),
            group(plus(no_chr(')'))),
            chr(')'),
            eof);
        if (SMatch<4> match; regex_match(expression, match, comparison_re)) {
            std::string_view left = match[1].str();
            auto op = match[2].parallel_index;
            std::string_view right = match[3].str();
            auto e_left = eval_recursion(left, globals, locals, block, asset_references, recursion + 1);
            auto e_right = eval_recursion(right, globals, locals, block, asset_references, recursion + 1);
            switch (op) {
                case 0: return e_left == e_right;
                case 1: return e_left != e_right;
                case 2: return e_left <= e_right;
                case 3: return e_left >= e_right;
                case 4: { // in
                    auto elems = e_right.get<std::set<nlohmann::json>>();
                    return elems.contains(e_left);
                }
                case 5: { // not in
                    auto elems = e_right.get<std::set<nlohmann::json>>();
                    return !elems.contains(e_left);
                }
                case 6: return e_left.get<double>() - e_right.get<double>();
            }
            verbose_abort("Unknown operator index: \"" + std::to_string(op) + "\". Line: \"" + std::string{expression} + '"');
        }
    }
    subst = [&subst_](const std::string_view& s){return Mlib::substitute_dollar(s, subst_);};
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
    if ((expression.size() >= 2) && (expression[0] == '{') && (expression[expression.size() - 1] == '}')) {
        // static const DECLARE_REGEX(set_re, "^\\{(.*)\\}$");
        // static const DECLARE_REGEX(comma_re, ", ");
        static const auto comma_re = par(str(", "), NC);
        std::set<nlohmann::json> result;
        auto rem = find_all_templated(expression.substr(1, expression.size() - 2), comma_re, [&](const SMatch<2>& match2b) {
            if (match2b[1].matched()) {
                if (!result.insert(eval_recursion(match2b[1].str(), globals, locals, block, asset_references, recursion + 1)).second) {
                    THROW_OR_ABORT("Duplicate element: \"" + std::string{ match2b[1].str() } + '"');
                }
            }
            });
        if (!rem.empty()) {
            THROW_OR_ABORT("Could not parse \"" + std::string(expression) + "\". Remainder: \"" + std::string(rem) + '"');
        }
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
        if (expression == "%true") {
            return true;
        }
        if (expression == "%false") {
            return false;
        }
        nlohmann::json var;
        if ((expression.length() > 1) && (expression[1] == '%')) {
            // static const DECLARE_REGEX(query_re, "^..([^/]+)/([^/]+)/(\\w+)$");
            static const auto query_re = seq(adot, adot, NSL, sl, NSL, sl, NSL, opt(seq(sl, NSL)), eof);
            SMatch<5> match;
            if (!regex_match(expression, match, query_re)) {
                THROW_OR_ABORT("Could not parse asset path: \"" + std::string{ expression } + '"');
            }
            auto asset_id = subst(match[DbQueryGroups::asset_id].str());
            const auto& db = asset_references[subst(match[DbQueryGroups::group].str())]
                .at(asset_id)
                .rp
                .database;
            if (match[DbQueryGroups::key].matched()) {
                auto res = db.at(subst(match[DbQueryGroups::value].str()));
                if (res.type() != nlohmann::detail::value_t::object) {
                    THROW_OR_ABORT("Database value is not of type object: \"" + std::string{ expression } + '"');
                }
                auto key = subst(match[DbQueryGroups::key].str());
                auto it = res.find(key);
                if (it == res.end()) {
                    THROW_OR_ABORT("Could not find database key \"" + std::string{ key } + "\": \"" + std::string{ expression } + "\". Asset ID: \"" + asset_id + "\".");
                }
                var = *it;
            } else {
                auto key = subst(match[DbQueryGroups::value].str());
                auto v = db.try_at(key);
                if (!v.has_value()) {
                    THROW_OR_ABORT("Could not find database key \"" + std::string{ key } + "\": \"" + std::string{ expression } + "\". Asset ID: \"" + asset_id + "\".");
                }
                var = *v;
            }
        } else if ((expression.length() > 1) && (expression[1] == '/')) {
            // static const DECLARE_REGEX(query_re, "^..([^/]+)/([^/]+)$");
            static const auto query_re = seq(adot, adot, NSL, sl, NSL, eof);
            SMatch<3> match;
            if (!regex_match(expression, match, query_re)) {
                THROW_OR_ABORT("Could not parse asset path: \"" + std::string{ expression } + '"');
            }
            auto dict_name = subst(match[DictQueryGroups::dict].str());
            const auto& dict = at(dict_name, globals.json(), locals.json(), block.json());
            if (dict.type() != nlohmann::detail::value_t::object) {
                THROW_OR_ABORT("Variable \"" + dict_name + "\" is not a dictionary");
            }
            auto key_name = subst(match[DictQueryGroups::key].str());
            var = JsonView{ dict }.at(key_name);
        } else {
            var = at(subst(expression.substr(1)), globals.json(), locals.json(), block.json());
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
    const JsonView& block,
    const AssetReferences& asset_references)
{
    return eval_recursion(expression, globals, locals, block, asset_references, 0);
}

nlohmann::json Mlib::eval(
    std::string_view expression,
    const JsonView& globals,
    const JsonView& locals,
    const AssetReferences& asset_references)
{
    return eval(
        expression,
        globals,
        locals,
        JsonView{ nlohmann::json::object() },
        asset_references);
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
    const JsonView& block,
    const AssetReferences& asset_references)
{
    auto result = eval(expression, globals, locals, block, asset_references);
    if (result.type() != nlohmann::detail::value_t::boolean) {
        THROW_OR_ABORT("Expression is not of type bool: \"" + std::string{ expression } + '"');
    }
    return result;
}

template <>
bool Mlib::eval<bool>(
    std::string_view expression,
    const JsonView& globals,
    const JsonView& locals,
    const AssetReferences& asset_references)
{
    return eval<bool>(
        expression,
        globals,
        locals,
        JsonView{ nlohmann::json::object() },
        asset_references);
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
    const JsonView& block,
    const AssetReferences& asset_references)
{
    auto result = eval(expression, globals, locals, block, asset_references);
    if (result.type() != nlohmann::detail::value_t::string) {
        THROW_OR_ABORT("Expression is not of type string: \"" + std::string{ expression } + '"');
    }
    return result;
}

template <>
std::string Mlib::eval<std::string>(
    std::string_view expression,
    const JsonView& globals,
    const JsonView& locals,
    const AssetReferences& asset_references)
{
    return eval<std::string>(
        expression,
        globals,
        locals,
        JsonView{ nlohmann::json::object() },
        asset_references);
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
