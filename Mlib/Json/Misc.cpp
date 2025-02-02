#include "Misc.hpp"
#include <Mlib/Strings/String.hpp>

using namespace Mlib;

std::string Mlib::get_multiline_string(const nlohmann::json& j) {
    return Mlib::join("\n", j.get<std::vector<std::string>>());
}

void Mlib::validate(const nlohmann::json& j, const std::set<std::string_view>& known_keys, std::string_view prefix) {
    if (j.type() != nlohmann::detail::value_t::object) {
        THROW_OR_ABORT(std::string{ prefix } + "JSON is not of type object");
    }
    for (const auto& [key, _] : j.items()) {
        if (!key.starts_with("#") && !known_keys.contains(key)) {
            THROW_OR_ABORT(std::string{ prefix } + "Unknown key in JSON: \"" + key + '"');
        }
    }
}

void Mlib::validate_complement(const nlohmann::json& j, const std::set<std::string_view>& known_keys, std::string_view prefix) {
    if (j.type() != nlohmann::detail::value_t::object) {
        THROW_OR_ABORT(std::string{ prefix } + "JSON is not of type object");
    }
    for (const auto& key : known_keys) {
        if (j.contains(key)) {
            THROW_OR_ABORT(std::string{ prefix } + "Key already exists in JSON: \"" + std::string{ key } + '"');
        }
    }
}
