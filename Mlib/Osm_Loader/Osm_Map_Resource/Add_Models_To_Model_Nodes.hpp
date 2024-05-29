#pragma once
#include <map>
#include <string>

namespace Mlib {

class BatchResourceInstantiator;
class GroundBvh;
class SceneNodeResources;
struct Node;
struct Way;

void add_models_to_model_nodes(
    BatchResourceInstantiator& bri,
    const GroundBvh& ground_bvh,
    const SceneNodeResources& resources,
    const std::map<std::string, Node>& nodes,
    const std::map<std::string, Way>& ways,
    const std::string& game_level);

}
