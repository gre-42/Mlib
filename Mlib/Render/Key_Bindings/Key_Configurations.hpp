#pragma once
#include <Mlib/Map/Map.hpp>
#include <string>

namespace Mlib {

struct KeyConfiguration;
class CursorStates;

class KeyConfigurations {
public:
    KeyConfigurations();
    ~KeyConfigurations();

    void load(
        const std::string& filename,
        const std::string& fallback_filename);

    void insert(std::string id, const KeyConfiguration& key_configuration);

    const KeyConfiguration& get(const std::string& name) const;
private:
    Map<std::string, KeyConfiguration> key_configurations_;
};

}
