#include "Json_Macro_Arguments.hpp"
#include <Mlib/Macro_Executor/Macro_Keys.hpp>
#include <Mlib/Regex.hpp>
#include <ostream>

using namespace Mlib;

JsonMacroArguments::JsonMacroArguments() = default;

JsonMacroArguments::JsonMacroArguments(nlohmann::json j)
: j_{std::move(j)}
{}

void JsonMacroArguments::set(const std::string& key, nlohmann::json value) {
    j_[key] = std::move(value);
}

void JsonMacroArguments::merge(const JsonMacroArguments& other, const std::string& prefix) {
    for (const auto& [key, value] : other.j_.items()) {
        j_[prefix + key] = value;
    }
}

static nlohmann::json subst_and_replace(const nlohmann::json& j, const nlohmann::json& replace) {
    auto subst = [&replace](const std::string& s) {
        auto it = replace.find(s);
        if (it == replace.end()) {
            THROW_OR_ABORT("Could not find variable with name \"" + s + '"');
        }
        if (it->type() != nlohmann::detail::value_t::string) {
            std::stringstream sstr;
            sstr << "Variable \"" << s << "\" is not of type string. Value: \"" << *it << '"';
            THROW_OR_ABORT(sstr.str());
        }
        return it->get<std::string>();
    };
    if (j.type() == nlohmann::detail::value_t::object) {
        nlohmann::json result;
        for (const auto& [key, value] : j.items()) {
            if (key == MacroKeys::literals) {
                result[key] = value;
            } else {
                result[key] = subst_and_replace(value, replace);
            }
        }
        return result;
    }
    if (j.type() == nlohmann::detail::value_t::array) {
        nlohmann::json result;
        for (const auto& value : j) {
            result.push_back(subst_and_replace(value, replace));
        }
        return result;
    }
    if (j.type() == nlohmann::detail::value_t::string) {
        auto s = j.get<std::string>();
        if (s.empty()) {
            return "";
        }
        if (s[0] == '%') {
            if ((s.length() > 1) && (s[1] == '!')) {
                return !replace.at(s.substr(2)).get<bool>();
            } else {
                return replace.at(s.substr(1));
            }
        }
        return Mlib::substitute_dollar(s, subst);
    }
    return j;
}

nlohmann::json JsonMacroArguments::subst_and_replace(const nlohmann::json& j) const {
    return ::subst_and_replace(j, j_);
}

void JsonMacroArguments::insert_json(const nlohmann::json& j) {
    if (j.type() == nlohmann::detail::value_t::null) {
        return;
    }
    if (j.type() != nlohmann::detail::value_t::object) {
        THROW_OR_ABORT("Cannot insert non-object type");
    }
    for (const auto& [key, value] : j.items()) {
        insert_json(key, value);
    }
}

void JsonMacroArguments::insert_json(const std::string& key, nlohmann::json j) {
    if (j_.contains(key)) {
        THROW_OR_ABORT("Multiple definitions of key \"" + key + '"');
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

bool JsonMacroArguments::contains(const std::string& name) const {
    return j_.contains(name);
}

bool JsonMacroArguments::contains_non_null(const std::string& name) const {
    return j_.contains(name) &&
           (j_.at(name).type() != nlohmann::detail::value_t::null);
}

std::optional<nlohmann::json> JsonMacroArguments::try_at(const std::string& name) const {
    return j_.contains(name)
        ? j_.at(name)
        : std::optional<nlohmann::json>();
}

nlohmann::json JsonMacroArguments::at(const std::string& name) const {
    return j_.at(name);
}

std::string JsonMacroArguments::path(const std::string& name) const {
    auto res = fpath_(at<std::string>(name));
    if (res.is_variable) {
        THROW_OR_ABORT('"' + name + "\" is a variable, not a path");
    }
    return res.path;
}

std::string JsonMacroArguments::path(const std::string& name, const std::string& deflt) const {
    return contains(name)
        ? path(name)
        : deflt;
}

FPath JsonMacroArguments::path_or_variable(const std::string& name) const {
    return fpath_(at<std::string>(name));
}

FPath JsonMacroArguments::try_path_or_variable(const std::string& name) const {
    if (!contains(name)) {
        return FPath{
            .is_variable = false,
            .path = ""};
    }
    return fpath_(at<std::string>(name));
}

std::list<std::string> JsonMacroArguments::path_list(const std::string& name) const {
    return fpathes_(at<std::string>(name));
}

std::vector<FPath> JsonMacroArguments::pathes_or_variables(const std::string& name) const {
    return at_vector<std::string>(name, fpath_);
}

std::vector<JsonMacroArguments> JsonMacroArguments::elements() const {
    return get_vector<nlohmann::json>([this](const nlohmann::json& j){return as_child(j);});
}

JsonMacroArguments JsonMacroArguments::child(const std::string& name) const {
    auto it = j_.find(name);
    if (it == j_.end()) {
        THROW_OR_ABORT("Could not find child arguments with name \"" + name + '"');
    }
    return as_child(*it);
}

std::optional<JsonMacroArguments> JsonMacroArguments::try_get_child(const std::string& name) const {
    auto it = j_.find(name);
    if (it == j_.end()) {
        return std::nullopt;
    }
    return as_child(*it);
}

JsonMacroArguments JsonMacroArguments::as_child(const nlohmann::json& j) const {
    JsonMacroArguments res{j};
    res.set_fpath(fpath_);
    res.set_fpathes(fpathes_);
    res.set_spath(spath_);
    return res;
}

std::string JsonMacroArguments::get_multiline_string() const {
    return Mlib::get_multiline_string(j_);
}

std::string JsonMacroArguments::at_multiline_string(const std::string& name) const {
    try {
        return Mlib::get_multiline_string(at(name));
    } catch (const nlohmann::json::type_error& e) {
        throw std::runtime_error("Could not interpret \"" + name + "\" as a multiline string: " + e.what());
    }
}

std::string JsonMacroArguments::at_multiline_string(const std::string& name, const std::string& default_) const {
    if (!contains(name)) {
        return default_;
    }
    return at_multiline_string(name);
}

void JsonMacroArguments::validate(const std::set<std::string>& allowed_attributes) const {
    Mlib::validate(j_, allowed_attributes);
}

std::ostream& Mlib::operator << (std::ostream& ostr, const JsonMacroArguments& arguments) {
    ostr << "JSON: " << arguments.j_ << '\n';
    return ostr;
}
