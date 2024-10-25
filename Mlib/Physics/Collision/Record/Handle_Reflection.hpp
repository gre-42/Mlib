#pragma once
#include <Mlib/Scene_Pos.hpp>
#include <cstddef>

namespace Mlib {

struct IntersectionScene;
template <typename TData, size_t... tshape>
class FixedArray;
class IIntersectionInfo;

void handle_reflection(
    const IntersectionScene& c,
    const IIntersectionInfo& iinfo,
    float surface_stiction_factor);

}
