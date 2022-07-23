#pragma once
#include <string>

namespace Mlib {

class ISupplyDepots;
class SceneNode;
struct RenderableResourceFilter;

struct InstantiationOptions {
    ISupplyDepots* supply_depots;
    const std::string& instance_name;
    SceneNode& scene_node;
    const RenderableResourceFilter& renderable_resource_filter;
};

}
