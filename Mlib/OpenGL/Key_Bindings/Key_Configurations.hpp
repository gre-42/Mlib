#pragma once
#include <Mlib/Map/Map.hpp>
#include <Mlib/Map/Unordered_Map.hpp>
#include <Mlib/Scene_Config/Remote_Integers.hpp>
#include <cstdint>
#include <string>

namespace Mlib {

struct KeyConfiguration;

class KeyConfigurations {
    KeyConfigurations(const KeyConfigurations&) = delete;
    KeyConfigurations& operator = (const KeyConfigurations&) = delete;
public:
    KeyConfigurations();
    ~KeyConfigurations();

    void load(
        NUserCountType user_id,
        const std::string& filename,
        const std::string& fallback_filename);

    void insert(NUserCountType user_id, std::string id, KeyConfiguration key_configuration);

    const KeyConfiguration& get(NUserCountType user_id, const std::string& id) const;
    const KeyConfiguration* try_get(NUserCountType user_id, const std::string& id) const;
private:
    UnorderedMap<uint32_t, Map<std::string, KeyConfiguration>> key_configurations_;
};

}
