#include "Damage_Source.hpp"
#include <Mlib/Throw_Or_Abort.hpp>
#include <map>
#include <nlohmann/json.hpp>
#include <string>

using namespace Mlib;

void Mlib::from_json(const nlohmann::json& j, DamageSource& damage_source) {
    damage_source = DamageSource::NONE;
    for (const auto& s : j.get<std::vector<std::string>>()) {
        static const std::map<std::string, DamageSource> m{
            {"crash", DamageSource::CRASH},
            {"bullet", DamageSource::BULLET},
            {"any", DamageSource::ANY}
        };
        auto it = m.find(s);
        if (it == m.end()) {
            THROW_OR_ABORT("Unknown damage source: \"" + s + '"');
        }
        damage_source |= it->second;
    }
}
