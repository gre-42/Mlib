#pragma once
#include <list>
#include <map>
#include <string>

namespace Mlib {

class BatchResourceInstantiator;
template <class TData, size_t... tshape>
class FixedArray;
class GroundBvh;
struct Node;
struct Way;
class SceneNodeResources;

void add_models_to_model_nodes(
    BatchResourceInstantiator& bri,
    const std::list<FixedArray<FixedArray<float, 2>, 2>>& way_segments,
    const GroundBvh& ground_bvh,
    const SceneNodeResources& resources,
    const std::map<std::string, Node>& nodes,
    const std::map<std::string, Way>& ways,
    float scale,
    const std::string& game_level);

}
