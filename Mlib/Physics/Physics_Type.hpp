#pragma once

namespace Mlib {

enum class PhysicsType {
    VERSION1,
    TRACKING_SPRINGS,
    BUILTIN
};

inline PhysicsType physics_type_from_string(const std::string& pyhsics_type) {
    if (pyhsics_type == "version1") {
        return PhysicsType::VERSION1;
    } else if (pyhsics_type == "tracking_springs") {
        return PhysicsType::TRACKING_SPRINGS;
    } else if (pyhsics_type == "builtin") {
        return PhysicsType::BUILTIN;
    } else {
        throw std::runtime_error("Unknown physics type");
    }
}

}
