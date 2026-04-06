#pragma once
#include <Mlib/Geometry/Material/Render_Pass.hpp>
#include <Mlib/Hashing/Variable_And_Hash.hpp>
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Scene_Graph/Remote_User_Filter.hpp>
#include <chrono>
#include <compare>
#include <cstdint>

namespace Mlib {

enum class ExternalRenderPassType;
class SceneNode;
class IRenderableScene;

struct ExternalRenderPass {
    RemoteObserver observer = RemoteObserver::all();
    ExternalRenderPassType pass = ExternalRenderPassType::NONE;
    std::chrono::steady_clock::time_point time;
    VariableAndHash<std::string> black_node_name;
    DanglingBaseClassPtr<SceneNode> singular_node = nullptr;
    DanglingBaseClassPtr<SceneNode> nonstandard_camera_node = nullptr;
    DanglingBaseClassPtr<IRenderableScene> renderable_scene = nullptr;
    std::strong_ordering operator <=> (const ExternalRenderPass&) const = default;
};

enum class InternalRenderPass {
    NONE = 0,
    INITIAL = 1 << 0,
    BLENDED_EARLY = 1 << 1,
    BLENDED_LATE = 1 << 2,
    AGGREGATE = 1 << 3,
    PARTICLES = 1 << 4,
    PRELOADED = AGGREGATE,
    ANY_BLENDED = BLENDED_EARLY | BLENDED_LATE
};

inline InternalRenderPass operator&(InternalRenderPass a, InternalRenderPass b) {
    return InternalRenderPass((int)a & (int)b);
}

inline bool any(InternalRenderPass a) {
    return a != InternalRenderPass::NONE;
}

}
