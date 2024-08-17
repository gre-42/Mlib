#pragma once
#include <cstddef>

namespace Mlib {

enum class PrimitiveDimensions: size_t {
    BEGIN = 2,
    LINE = 2,
    TRIANGLE = 3,
    QUAD = 4,
    END = 5
};

inline consteval PrimitiveDimensions operator + (PrimitiveDimensions a, size_t b) {
    return (PrimitiveDimensions)((size_t)a + b);
}

}
