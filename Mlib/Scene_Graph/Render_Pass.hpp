#pragma once
#include <Mlib/String.hpp>

namespace Mlib {

enum class ExternalRenderPassType {
    UNDEFINED,
    STANDARD_WITH_POSTPROCESSING,
    STANDARD_WO_POSTPROCESSING,
    LIGHTMAP_TO_TEXTURE,
    DIRTMAP
};

struct ExternalRenderPass {
    ExternalRenderPassType pass;
    const std::string black_node_name;
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
