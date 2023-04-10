#include "Json_Macro_Arguments.hpp"
#include <ostream>

using namespace Mlib;

JsonMacroArguments::JsonMacroArguments() = default;

JsonMacroArguments::JsonMacroArguments(nlohmann::json j)
: j_{std::move(j)}
{}

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

void JsonMacroArguments::insert_path(const std::string& key, std::string value) {
    if (!pathes_.try_emplace(key, std::move(value)).second) {
        THROW_OR_ABORT("Path with name \"" + key + "\" already exists");
    }
}

void JsonMacroArguments::insert_path_list(const std::string& key, std::list<std::string> value) {
    if (!path_lists_.try_emplace(key, std::move(value)).second) {
        THROW_OR_ABORT("Path list with name \"" + key + "\" already exists");
    }
}

void JsonMacroArguments::insert_child(const std::string& key, JsonMacroArguments value) {
    if (!children_.try_emplace(key, std::move(value)).second) {
        THROW_OR_ABORT("Child arguments with name \"" + key + "\" already exist");
    }
}

bool JsonMacroArguments::contains_json(const std::string& name) const {
    return j_.contains(name);
}

nlohmann::json JsonMacroArguments::at(const std::string& name) const {
    return j_.at(name);
}

const std::string& JsonMacroArguments::path(const std::string& name) const {
    auto it = pathes_.find(name);
    if (it == pathes_.end()) {
        THROW_OR_ABORT("Could not find path with name \"" + name + '"');
    }
    return it->second;
}

const std::list<std::string>& JsonMacroArguments::path_list(const std::string& name) const {
    auto it = path_lists_.find(name);
    if (it == path_lists_.end()) {
        THROW_OR_ABORT("Could not find path-list with name \"" + name + '"');
    }
    return it->second;
}

const JsonMacroArguments& JsonMacroArguments::child(const std::string& name) const {
    auto it = children_.find(name);
    if (it == children_.end()) {
        THROW_OR_ABORT("Could not find child arguments with name \"" + name + '"');
    }
    return it->second;
}

bool JsonMacroArguments::contains_path(const std::string& name) const {
    return pathes_.contains(name);
}

bool JsonMacroArguments::contains_path_list(const std::string& name) const {
    return path_lists_.contains(name);
}

bool JsonMacroArguments::contains_child(const std::string& name) const {
    return children_.contains(name);
}

void JsonMacroArguments::validate(const std::set<std::string>& allowed_attributes) const {
    for (const auto& [key, _] : j_.items()) {
        if (!allowed_attributes.contains(key)) {
            THROW_OR_ABORT("Unknown key in JSON: \"" + key + '"');
        }
    }
    for (const auto& [key, _] : pathes_) {
        if (!allowed_attributes.contains(key)) {
            THROW_OR_ABORT("Unknown path variable: \"" + key + '"');
        }
    }
    for (const auto& [key, _] : path_lists_) {
        if (!allowed_attributes.contains(key)) {
            THROW_OR_ABORT("Unknown path-list variable: \"" + key + '"');
        }
    }
}

std::ostream& Mlib::operator << (std::ostream& ostr, const JsonMacroArguments& arguments) {
    ostr << "JSON: " << arguments.j_ << '\n';
    ostr << "Pathes:\n";
    for (const auto& [key, value] : arguments.pathes_) {
        ostr << key << " -> " << value << '\n';
    }
    ostr << "Path-lists:\n";
    for (const auto& [key, value] : arguments.path_lists_) {
        ostr << key << '\n';
        for (const auto& p : value) {
            ostr << "  " << p << '\n';
        }
    }
    return ostr;
}
