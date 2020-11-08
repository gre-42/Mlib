#pragma once

namespace Mlib {

enum class PhysicsType {
    VERSION1,
    STICKY_SPRINGS,
    TRACKING_SPRINGS
};

inline PhysicsType physics_type_from_string(const std::string& pyhsics_type) {
    if (pyhsics_type == "version1") {
        return PhysicsType::VERSION1;
    } else if (pyhsics_type == "sticky_springs") {
        return PhysicsType::STICKY_SPRINGS;
    } else if (pyhsics_type == "tracking_springs") {
        return PhysicsType::TRACKING_SPRINGS;
    } else {
        throw std::runtime_error("Unknown physics type");
    }
}

}
