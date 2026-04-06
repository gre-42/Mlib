#pragma once
#include <Mlib/Json/Base.hpp>
#include <map>
#include <stdexcept>

namespace Mlib {

template <typename TKey, typename TValue>
void from_json(const nlohmann::json& j, std::map<TKey, TValue>& m) {
    if (!j.is_object()) {
        throw nlohmann::detail::type_error::create(302, nlohmann::detail::concat("type must be object, but is ", j.type_name()), &j);
    }
    m.clear();
    for (const auto& [key, value] : j.items()) {
        TKey k;
        TValue v;
        from_json(key, k);
        from_json(value, v);
        if (!m.try_emplace(std::move(k), std::move(v)).second) {
            throw std::runtime_error("JSON dictionary contains duplicate keys after conversion");
        }
    }
}

}
