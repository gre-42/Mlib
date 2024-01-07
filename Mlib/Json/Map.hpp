#pragma once
#include <Mlib/Throw_Or_Abort.hpp>
#include <map>
#include <nlohmann/json.hpp>

namespace Mlib {

template <typename TKey, typename TValue>
void from_json(const nlohmann::json& j, std::map<TKey, TValue>& m) {
    if (!j.is_object()) {
        THROW_OR_ABORT2(nlohmann::detail::type_error::create(302, nlohmann::detail::concat("type must be object, but is ", j.type_name()), &j));
    }
    m.clear();
    for (const auto& [key, value] : j.items()) {
        TKey k;
        TValue v;
        from_json(key, k);
        from_json(value, v);
        if (!m.try_emplace(std::move(k), std::move(v)).second) {
            THROW_OR_ABORT("JSON dictionary contains duplicate keys after conversion");
        }
    }
}

}
