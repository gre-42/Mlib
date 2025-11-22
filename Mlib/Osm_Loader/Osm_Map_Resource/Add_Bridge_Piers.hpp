#pragma once
#include <Mlib/Scene_Config/Scene_Precision.hpp>
#include <list>
#include <map>
#include <memory>
#include <string>

namespace Mlib {

template <class T>
class VariableAndHash;
class SceneNodeResources;
template <class TPos>
class TriangleList;
class GroundBvh;
struct Node;
struct Way;
struct Material;
struct Morphology;

void add_bridge_piers(
    std::list<std::shared_ptr<TriangleList<CompressedScenePos>>>& tls_bridge_piers,
    Material material,
    const Morphology& morphology,
    SceneDir bridge_pier_radius,
    const SceneNodeResources& scene_node_resources,
    const VariableAndHash<std::string>& model_name,
    const GroundBvh& ground_bvh,
    const std::map<std::string, Node>& nodes,
    const std::map<std::string, Way>& ways);

}
