#pragma once
#include <Mlib/Scene_Precision.hpp>
#include <list>
#include <map>
#include <memory>
#include <string>

namespace Mlib {

template <class T>
class VariableAndHash;
struct Material;
struct Morphology;
template <class TPos>
class TriangleList;
template <class TData, size_t... tshape>
class OrderableFixedArray;
template <typename TData, size_t... tshape>
class FixedArray;
struct Building;
struct Node;
class SceneNodeResources;
class GetMorphology;

void draw_roofs(
    std::list<std::shared_ptr<TriangleList<CompressedScenePos>>>& tls,
    const SceneNodeResources& scene_node_resources,
    const VariableAndHash<std::string>& model_name,
    const std::map<OrderableFixedArray<CompressedScenePos, 2>, FixedArray<CompressedScenePos, 3>>& displacements,
    const Material& roof_material,
    const Material& rail_material,
    const GetMorphology& get_morphology,
    const FixedArray<float, 3>& color,
    const std::list<Building>& buildings,
    const std::map<std::string, Node>& nodes,
    float scale,
    float uv_scale,
    float max_length);

}
