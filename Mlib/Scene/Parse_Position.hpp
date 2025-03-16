#pragma once
#include <Mlib/Json/Base.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <cstddef>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
class SceneNodeResources;

FixedArray<CompressedScenePos, 3> parse_position(
    const nlohmann::json& j,
    SceneNodeResources& scene_node_resources);

}
