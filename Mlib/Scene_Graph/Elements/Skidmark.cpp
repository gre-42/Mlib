#include "Skidmark.hpp"
#include <Mlib/Array/Fixed_Array_Hash.hpp>
#include <Mlib/Scene_Graph/Culling/Visibility_Check.hpp>

using namespace Mlib;

size_t Skidmark::shading_hash() const {
    return hash_combine(
        particle_type,
        VisibilityCheck{vp}.orthographic());
}
