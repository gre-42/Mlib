#pragma once
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <compare>

namespace Mlib {

enum class ExternalRenderPassType;
class SceneNode;

struct ExternalRenderPass {
    ExternalRenderPassType pass;
    std::string black_node_name;
    DanglingPtr<SceneNode> singular_node = nullptr;
    DanglingPtr<SceneNode> camera_node = nullptr;
    std::strong_ordering operator <=> (const ExternalRenderPass&) const = default;
};

enum class InternalRenderPass {
    INITIAL,
    BLENDED,
    AGGREGATE
};

struct RenderPass {
    const ExternalRenderPass external;
    const InternalRenderPass internal;
};

}
