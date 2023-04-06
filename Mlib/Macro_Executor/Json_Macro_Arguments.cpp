#include "Json_Macro_Arguments.hpp"
#include <ostream>

using namespace Mlib;

void JsonMacroArguments::insert_json(const nlohmann::json& j) {
    if (j.type() != nlohmann::detail::value_t::object) {
        THROW_OR_ABORT("Cannot insert non-object type");
    }
    for (const auto& [key, value] : j.items()) {
        if (j_.contains(key)) {
            THROW_OR_ABORT("Multiple definitions of key \"" + key + '"');
        }
        j_[key] = value;
    }
}

void JsonMacroArguments::insert_script(const std::string& key, const std::string& value) {
    if (!scripts_.try_emplace(key, value).second) {
        THROW_OR_ABORT("Script with name \"" + key + "\" already exists");
    }
}

void JsonMacroArguments::insert_path(const std::string& key, const std::string& value) {
    if (!pathes_.try_emplace(key, value).second) {
        THROW_OR_ABORT("Path with name \"" + key + "\" already exists");
    }
}
void JsonMacroArguments::insert_path_list(const std::string& key, const std::list<std::string>& value) {
    if (!path_lists_.try_emplace(key, value).second) {
        THROW_OR_ABORT("Path list with name \"" + key + "\" already exists");
    }
}

bool JsonMacroArguments::contains_json(const std::string& name) const {
    return j_.contains(name);
}

const std::string& JsonMacroArguments::script(const std::string& name) const {
    auto it = scripts_.find(name);
    if (it == scripts_.end()) {
        THROW_OR_ABORT("Could not find script with name \"" + name + '"');
    }
    return it->second;
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

std::ostream& Mlib::operator << (std::ostream& ostr, const JsonMacroArguments& arguments) {
    ostr << "JSON: " << arguments.j_ << '\n';
    ostr << "Scripts:\n";
    for (const auto& [key, value] : arguments.scripts_) {
        ostr << key << " -> " << value << '\n';
    }
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
