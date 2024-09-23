#pragma once
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Scene_Graph/Pose_Interpolation_Mode.hpp>
#include <string>

namespace Mlib {

class SceneNode;
struct RenderableResourceFilter;
class RenderingResources;
template <class T>
class VariableAndHash;

struct ChildInstantiationOptions {
    RenderingResources* rendering_resources = nullptr;
    const VariableAndHash<std::string>& instance_name;
    DanglingRef<SceneNode> scene_node;
    PoseInterpolationMode interpolation_mode = PoseInterpolationMode::UNDEFINED;
    const RenderableResourceFilter& renderable_resource_filter;
};

}
