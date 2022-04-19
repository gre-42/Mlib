#pragma once
#include <string>

namespace Mlib {

enum class ExternalRenderPassType {
    NONE                            = 0,
    STANDARD                        = 1,
    DIRTMAP                         = 2,
    LIGHTMAP_ANY_MASK               = (1 << 16),
    LIGHTMAP_DEPTH_MASK             = (1 << 17),
    LIGHTMAP_COLOR_MASK             = (1 << 18),
    LIGHTMAP_EMITS_COLORS_MASK      = (1 << 19),
    LIGHTMAP_IS_BLACK_MASK          = (1 << 20),
    LIGHTMAP_DEPTH                  = (LIGHTMAP_ANY_MASK | LIGHTMAP_DEPTH_MASK | LIGHTMAP_EMITS_COLORS_MASK) + 0,
    LIGHTMAP_GLOBAL_STATIC          = (LIGHTMAP_ANY_MASK | LIGHTMAP_COLOR_MASK | LIGHTMAP_EMITS_COLORS_MASK) + 0,
    LIGHTMAP_GLOBAL_DYNAMIC         = (LIGHTMAP_ANY_MASK | LIGHTMAP_COLOR_MASK | LIGHTMAP_EMITS_COLORS_MASK) + 1,
    LIGHTMAP_BLACK_GLOBAL_STATIC    = (LIGHTMAP_ANY_MASK | LIGHTMAP_COLOR_MASK | LIGHTMAP_IS_BLACK_MASK) + 0,
    LIGHTMAP_BLACK_LOCAL_INSTANCES  = (LIGHTMAP_ANY_MASK | LIGHTMAP_COLOR_MASK | LIGHTMAP_IS_BLACK_MASK) + 1,
    LIGHTMAP_BLACK_NODE             = (LIGHTMAP_ANY_MASK | LIGHTMAP_COLOR_MASK | LIGHTMAP_IS_BLACK_MASK) + 2
};

inline ExternalRenderPassType operator & (ExternalRenderPassType a, ExternalRenderPassType b) {
    return (ExternalRenderPassType)((int)a & (int)b);
}

ExternalRenderPassType external_render_pass_type_from_string(const std::string& str);
std::string external_render_pass_type_to_string(ExternalRenderPassType pass);

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
