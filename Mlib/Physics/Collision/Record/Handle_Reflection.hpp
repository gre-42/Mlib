#pragma once
#include <Mlib/Scene_Config/Scene_Precision.hpp>
#include <cstddef>

namespace Mlib {

struct IntersectionScene;
template <typename TData, size_t... tshape>
class FixedArray;
struct IntersectionInfo;

void handle_reflection(
    const IntersectionScene& c,
    const IntersectionInfo& iinfo,
    float surface_stiction_factor);

}
