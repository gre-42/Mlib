#pragma once
#include <string>

namespace Mlib {

enum class CollidableMode {
    NONE = 0,
    COLLIDE = 1 << 0,
    MOVE = 1 << 1
};

CollidableMode collidable_mode_from_string(const std::string& mode);

inline bool any(CollidableMode m) {
    return m != CollidableMode::NONE;
}

inline CollidableMode operator & (CollidableMode a, CollidableMode b) {
    return (CollidableMode)((int)a & (int)b);
}

inline CollidableMode operator | (CollidableMode a, CollidableMode b) {
    return (CollidableMode)((int)a | (int)b);
}

inline CollidableMode& operator |= (CollidableMode& a, CollidableMode b) {
    (int&)a |= (int)b;
    return a;
}

}
