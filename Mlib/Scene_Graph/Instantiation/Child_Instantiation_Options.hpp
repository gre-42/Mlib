#pragma once
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <string>

namespace Mlib {

class SceneNode;
struct RenderableResourceFilter;
class RenderingResources;

struct ChildInstantiationOptions {
    RenderingResources* rendering_resources = nullptr;
    const std::string& instance_name;
    DanglingRef<SceneNode> scene_node;
    const RenderableResourceFilter& renderable_resource_filter;
};

}
