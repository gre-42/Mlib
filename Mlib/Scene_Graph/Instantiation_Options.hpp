#pragma once
#include <string>

namespace Mlib {

class IImpostors;
class ISupplyDepots;
class SceneNode;
struct RenderableResourceFilter;

struct InstantiationOptions {
    IImpostors* impostors = nullptr;
    ISupplyDepots* supply_depots = nullptr;
    const std::string& instance_name;
    SceneNode& scene_node;
    const RenderableResourceFilter& renderable_resource_filter;
};

}
