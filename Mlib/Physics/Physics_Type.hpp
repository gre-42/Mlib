#pragma once

namespace Mlib {

enum class PhysicsType {
    VERSION1,
    N_SPRINGS,
    TWO_SPRINGS
};

inline PhysicsType physics_type_from_string(const std::string& pyhsics_type) {
    if (pyhsics_type == "version1") {
        return PhysicsType::VERSION1;
    } else if (pyhsics_type == "n_springs") {
        return PhysicsType::N_SPRINGS;
    } else if (pyhsics_type == "two_springs") {
        return PhysicsType::TWO_SPRINGS;
    } else {
        throw std::runtime_error("Unknown physics type");
    }
}

}
