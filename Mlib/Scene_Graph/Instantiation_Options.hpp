#pragma once
#include <string>

namespace Mlib {

class IImposters;
class ISupplyDepots;
class SceneNode;
struct RenderableResourceFilter;

struct InstantiationOptions {
    IImposters* imposters = nullptr;
    ISupplyDepots* supply_depots = nullptr;
    const std::string& instance_name;
    SceneNode& scene_node;
    const RenderableResourceFilter& renderable_resource_filter;
};

}
