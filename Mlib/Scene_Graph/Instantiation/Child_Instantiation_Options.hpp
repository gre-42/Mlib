#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Scene_Graph/Pose_Interpolation_Mode.hpp>
#include <chrono>
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
    DanglingBaseClassRef<SceneNode> scene_node;
    std::optional<std::chrono::steady_clock::time_point> time;
    PoseInterpolationMode interpolation_mode = PoseInterpolationMode::UNDEFINED;
    const RenderableResourceFilter& renderable_resource_filter;
};

}
