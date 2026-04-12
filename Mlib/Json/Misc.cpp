
#include "Misc.hpp"
#include <Mlib/Strings/Str.hpp>
#include <Mlib/Strings/String.hpp>

using namespace Mlib;

void Mlib::from_json(const nlohmann::json& j, Utf8Path& v) {
    v = U8::u8str(json_get<std::string>(j));
}

std::string Mlib::get_multiline_string(const nlohmann::json& j) {
    return Mlib::join("\n", j.get<std::vector<std::string>>());
}

void Mlib::validate(const nlohmann::json& j, const std::set<std::string_view>& known_keys, std::string_view prefix) {
    if (j.type() != nlohmann::detail::value_t::object) {
        throw std::runtime_error(std::string{ prefix } + "JSON is not of type object");
    }
    for (const auto& [key, _] : j.items()) {
        if (!key.starts_with("#") && !known_keys.contains(key)) {
            throw std::runtime_error(std::string{ prefix } + "Unknown key in JSON: \"" + key + '"');
        }
    }
}

void Mlib::validate_complement(const nlohmann::json& j, const std::set<std::string_view>& known_keys, std::string_view prefix) {
    if (j.type() != nlohmann::detail::value_t::object) {
        throw std::runtime_error(std::string{ prefix } + "JSON is not of type object");
    }
    for (const auto& key : known_keys) {
        if (j.contains(key)) {
            throw std::runtime_error(std::string{ prefix } + "Key already exists in JSON: \"" + std::string{ key } + '"');
        }
    }
}
