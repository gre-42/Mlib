#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Variable_And_Hash.hpp>
#include <chrono>
#include <compare>
#include <cstdint>

namespace Mlib {

enum class ExternalRenderPassType;
class SceneNode;
class IRenderableScene;

struct ExternalRenderPass {
    uint32_t user_id;
    ExternalRenderPassType pass;
    std::chrono::steady_clock::time_point time;
    VariableAndHash<std::string> black_node_name;
    DanglingPtr<SceneNode> singular_node = nullptr;
    DanglingPtr<SceneNode> nonstandard_camera_node = nullptr;
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
    PRELOADED = AGGREGATE | PARTICLES,
    ANY_BLENDED = BLENDED_EARLY | BLENDED_LATE
};

inline InternalRenderPass operator&(InternalRenderPass a, InternalRenderPass b) {
    return InternalRenderPass((int)a & (int)b);
}

inline bool any(InternalRenderPass a) {
    return a != InternalRenderPass::NONE;
}

}
