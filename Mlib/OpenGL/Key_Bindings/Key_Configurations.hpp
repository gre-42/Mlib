#pragma once
#include <Mlib/Map/Map.hpp>
#include <Mlib/Map/Unordered_Map.hpp>
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
        uint32_t user_id,
        const std::string& filename,
        const std::string& fallback_filename);

    void insert(uint32_t user_id, std::string id, KeyConfiguration key_configuration);

    const KeyConfiguration& get(uint32_t user_id, const std::string& id) const;
    const KeyConfiguration* try_get(uint32_t user_id, const std::string& id) const;
private:
    UnorderedMap<uint32_t, Map<std::string, KeyConfiguration>> key_configurations_;
};

}
