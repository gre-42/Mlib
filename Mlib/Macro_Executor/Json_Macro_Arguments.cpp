#include "Json_Macro_Arguments.hpp"
#include <Mlib/Macro_Executor/Json_Expression.hpp>
#include <Mlib/Macro_Executor/Macro_Keys.hpp>
#include <ostream>

using namespace Mlib;

JsonMacroArguments::JsonMacroArguments()
    : JsonView{ j_, CheckIsObjectBehavior::NO_CHECK }
    , j_(nlohmann::json::value_t::object)
{}

JsonMacroArguments::JsonMacroArguments(const JsonMacroArguments& other)
    : JsonView{ j_, CheckIsObjectBehavior::NO_CHECK }
    , j_(other.j_)
    , fpathes_{ other.fpathes_ }
    , fpath_{ other.fpath_ }
    , spath_{ other.spath_ }
{
    if (j_.type() != nlohmann::detail::value_t::object) {
        THROW_OR_ABORT("JSON is not of type object");
    }
}

JsonMacroArguments::JsonMacroArguments(JsonMacroArguments&& other) noexcept
    : JsonView{ j_, CheckIsObjectBehavior::NO_CHECK }
    , j_(std::move(other.j_))
    , fpathes_{ std::move(other.fpathes_) }
    , fpath_{ std::move(other.fpath_) }
    , spath_{ std::move(other.spath_) }
{
    if (j_.type() != nlohmann::detail::value_t::object) {
        verbose_abort("JSON is not of type object");
    }
}

JsonMacroArguments::JsonMacroArguments(nlohmann::json j)
    : JsonView{ j_, CheckIsObjectBehavior::NO_CHECK }
    , j_(std::move(j))
{
    if (j_.type() != nlohmann::detail::value_t::object) {
        THROW_OR_ABORT("JSON is not of type object");
    }
}

JsonMacroArguments::JsonMacroArguments(
    const nlohmann::json& j,
    const std::set<std::string>& except)
    : JsonView{ j_, CheckIsObjectBehavior::NO_CHECK }
    , j_(nlohmann::json::object())
{
    if (j.type() != nlohmann::detail::value_t::object) {
        THROW_OR_ABORT("JSON is not of type object");
    }
    for (const auto& [k, v] : j.items()) {
        if (!except.contains(k)) {
            j_[k] = v;
        }
    }
}

JsonMacroArguments::~JsonMacroArguments() = default;

void JsonMacroArguments::set(std::string_view key, nlohmann::json value) {
    j_[key] = std::move(value);
}

void JsonMacroArguments::merge(const JsonMacroArguments& other, std::string_view prefix) {
    if (prefix.empty()) {
        for (const auto& [key, value] : other.j_.items()) {
            j_[key] = value;
        }
    } else {
        auto sp = std::string{ prefix };
        for (const auto& [key, value] : other.j_.items()) {
            j_[sp + key] = value;
        }
    }
}

static nlohmann::json subst_and_replace(
    const nlohmann::json& j,
    const nlohmann::json& globals,
    const nlohmann::json& locals,
    const AssetReferences& asset_references)
{
    if (j.type() == nlohmann::detail::value_t::object) {
        auto result = nlohmann::json::object();
        for (const auto& [key, value] : j.items()) {
            if (key.starts_with('#') ||
                unexpanded_keys.contains(key))
            {
                result[key] = value;
            } else {
                result[key] = subst_and_replace(value, globals, locals, asset_references);
            }
        }
        return result;
    }
    if (j.type() == nlohmann::detail::value_t::array) {
        auto result = nlohmann::json::array();
        for (const auto& value : j) {
            result.push_back(subst_and_replace(value, globals, locals, asset_references));
        }
        return result;
    }
    if (j.type() == nlohmann::detail::value_t::string) {
        auto s = j.get<std::string>();
        return eval(s, JsonView{ globals }, JsonView{ locals }, asset_references);
    }
    return j;
}

nlohmann::json JsonMacroArguments::subst_and_replace(
    const nlohmann::json& j,
    const nlohmann::json& globals,
    const AssetReferences& asset_references) const
{
    return ::subst_and_replace(j, globals, j_, asset_references);
}

void JsonMacroArguments::insert_json(
    const nlohmann::json& j,
    const std::set<std::string>& except)
{
    if (j.type() != nlohmann::detail::value_t::object) {
        THROW_OR_ABORT("Cannot insert non-object type");
    }
    for (const auto& [key, value] : j.items()) {
        if (!except.contains(key)) {
            insert_json(key, value);
        }
    }
}

void JsonMacroArguments::insert_json(std::string_view key, nlohmann::json j) {
    if (j_.contains(key)) {
        THROW_OR_ABORT("Multiple definitions of key \"" + std::string{ key } + '"');
    }
    j_[key] = std::move(j);
}

void JsonMacroArguments::set_fpathes(const std::function<std::list<std::string>(const std::filesystem::path& f)>& fpathes) {
    if (fpathes_) {
        THROW_OR_ABORT("fpathes already set");
    }
    fpathes_ = fpathes;
}

void JsonMacroArguments::set_fpath(const std::function<FPath(const std::filesystem::path& f)>& fpath) {
    if (fpath_) {
        THROW_OR_ABORT("fpath already set");
    }
    fpath_ = fpath;
}

void JsonMacroArguments::set_spath(const std::function<std::string(const std::filesystem::path& f)>& spath) {
    if (spath_) {
        THROW_OR_ABORT("spath already set");
    }
    spath_ = spath;
}

std::string JsonMacroArguments::path(std::string_view name) const {
    auto res = fpath_(at<std::string>(name));
    if (res.is_variable) {
        THROW_OR_ABORT('"' + std::string{ name } + "\" is a variable, not a path");
    }
    return res.path;
}

std::string JsonMacroArguments::path(std::string_view name, std::string_view deflt) const {
    return contains_non_null(name)
        ? std::string{ path(name) }
        : std::string{ deflt };
}

FPath JsonMacroArguments::path_or_variable(std::string_view name) const {
    return fpath_(at<std::string>(name));
}

FPath JsonMacroArguments::try_path_or_variable(std::string_view name) const {
    if (!contains(name)) {
        return FPath{
            .is_variable = false,
            .path = ""};
    }
    return fpath_(at<std::string>(name));
}

std::list<std::string> JsonMacroArguments::path_list(std::string_view name) const {
    return fpathes_(at<std::string>(name));
}

std::vector<FPath> JsonMacroArguments::pathes_or_variables(std::string_view name) const {
    return at_vector<std::string>(name, fpath_);
}

std::vector<FPath> JsonMacroArguments::try_pathes_or_variables(
    std::string_view name) const
{
    return contains_non_null(name)
        ? pathes_or_variables(name)
        : std::vector<FPath>();
}

std::string JsonMacroArguments::spath(std::string_view name) const {
    return spath_(at<std::string>(name));
}

std::vector<JsonMacroArguments> JsonMacroArguments::children(std::string_view name) const {
    auto el = at(name);
    if (el.type() != nlohmann::detail::value_t::array) {
        THROW_OR_ABORT("Not an array: \"" + std::string{ name } + '"');
    }
    return Mlib::get_vector<nlohmann::json>(el, [this](const nlohmann::json& c){return as_child(c);});
}

JsonMacroArguments JsonMacroArguments::child(std::string_view name) const {
    auto it = j_.find(name);
    if (it == j_.end()) {
        THROW_OR_ABORT("Could not find child arguments with name \"" + std::string{ name } + '"');
    }
    return as_child(*it);
}

std::optional<JsonMacroArguments> JsonMacroArguments::try_get_child(std::string_view name) const {
    auto it = j_.find(name);
    if (it == j_.end()) {
        return std::nullopt;
    }
    return as_child(*it);
}

JsonMacroArguments JsonMacroArguments::as_child(const nlohmann::json& j) const {
    JsonMacroArguments res{ j };
    res.set_fpath(fpath_);
    res.set_fpathes(fpathes_);
    res.set_spath(spath_);
    return res;
}

std::string JsonMacroArguments::get_multiline_string() const {
    return Mlib::get_multiline_string(j_);
}

std::string JsonMacroArguments::at_multiline_string(std::string_view name) const {
    try {
        return Mlib::get_multiline_string(at(name));
    } catch (const nlohmann::json::type_error& e) {
        throw std::runtime_error("Could not interpret \"" + std::string{ name } + "\" as a multiline string: " + e.what());
    }
}

std::string JsonMacroArguments::at_multiline_string(std::string_view name, std::string_view default_) const {
    if (!contains(name)) {
        return std::string{ default_ };
    }
    return at_multiline_string(name);
}
