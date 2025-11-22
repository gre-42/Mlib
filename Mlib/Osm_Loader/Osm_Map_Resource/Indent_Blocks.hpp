#pragma once
#include <Mlib/Scene_Config/Scene_Precision.hpp>

namespace Mlib {

struct NodesAndWays;

enum class ConnectorIndentation {
    FIXED,
    MOVING
};

NodesAndWays indent_blocks(
    const NodesAndWays& naws,
    CompressedScenePos amount,
    ConnectorIndentation connector_indentation);

}
