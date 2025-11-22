#pragma once
#include <Mlib/Scene_Config/Scene_Precision.hpp>
#include <cstddef>
#include <nlohmann/json_fwd.hpp>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
class SceneNodeResources;

FixedArray<CompressedScenePos, 3> parse_position(
    const nlohmann::json& j,
    SceneNodeResources& scene_node_resources);

}
