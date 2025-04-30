#pragma once
#include <Mlib/Scene_Precision.hpp>

namespace Mlib {

struct NodesAndWays;

NodesAndWays indent_buildings(
    const NodesAndWays& naws,
    CompressedScenePos amount);

}
