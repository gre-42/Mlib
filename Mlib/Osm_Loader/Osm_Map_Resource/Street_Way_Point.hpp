#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <utility>

namespace Mlib {

enum class WayPointLocation;

struct StreetWayPoint {
    std::pair<double, double> alpha;
    std::pair<FixedArray<CompressedScenePos, 3>, FixedArray<CompressedScenePos, 3>> edge;
    WayPointLocation location;
    FixedArray<CompressedScenePos, 3> position() const {
        return (
            alpha.first * edge.first.casted<ScenePos>() +
            alpha.second * edge.second.casted<ScenePos>())
            .casted<CompressedScenePos>();
    }
};

}
