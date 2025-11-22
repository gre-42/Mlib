#pragma once
#include <Mlib/Scene_Config/Scene_Precision.hpp>

namespace Mlib {

template <class TRegionType, class TGeometry>
struct RegionWithMargin {
    TRegionType hole_type;
    TRegionType margin_type;
    CompressedScenePos margin;
    TGeometry geometry;
};

}
