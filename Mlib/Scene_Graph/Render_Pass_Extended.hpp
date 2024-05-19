#pragma once
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <chrono>
#include <compare>

namespace Mlib {

enum class ExternalRenderPassType;
class SceneNode;

struct ExternalRenderPass {
    ExternalRenderPassType pass;
    std::chrono::steady_clock::time_point time;
    std::string black_node_name;
    DanglingPtr<SceneNode> singular_node = nullptr;
    DanglingPtr<SceneNode> nonstandard_camera_node = nullptr;
    std::strong_ordering operator <=> (const ExternalRenderPass&) const = default;
};

enum class InternalRenderPass {
    NONE = 0,
    INITIAL = 1 << 0,
    BLENDED = 1 << 1,
    AGGREGATE = 1 << 2,
    PARTICLES = 1 << 3,
    PRELOADED = AGGREGATE | PARTICLES
};

inline InternalRenderPass operator&(InternalRenderPass a, InternalRenderPass b) {
    return InternalRenderPass((int)a & (int)b);
}

inline bool any(InternalRenderPass a) {
    return a != InternalRenderPass::NONE;
}

struct RenderPass {
    const ExternalRenderPass external;
    const InternalRenderPass internal;
};

}
