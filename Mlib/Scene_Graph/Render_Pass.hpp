#pragma once
#include <string>

namespace Mlib {

enum class ExternalRenderPassType {
    NONE                                = 0,
    STANDARD                            = (1 << 0),
    DIRTMAP                             = (1 << 1),

    LIGHTMAP_ANY_MASK                   = (1 << 2),
    LIGHTMAP_DEPTH_MASK                 = (1 << 3),
    LIGHTMAP_COLOR_MASK                 = (1 << 4),
    LIGHTMAP_EMITS_COLORS_MASK          = (1 << 5),
    LIGHTMAP_IS_BLACK_MASK              = (1 << 6),
    LIGHTMAP_IS_STATIC_MASK             = (1 << 7),
    LIGHTMAP_IS_DYNAMIC_MASK            = (1 << 8),

    LIGHTMAP_GLOBAL_STATIC_MASK         = (1 << 9),
    LIGHTMAP_GLOBAL_DYNAMIC_MASK        = (1 << 10),
    LIGHTMAP_BLACK_GLOBAL_STATIC_MASK   = (1 << 11),
    LIGHTMAP_BLACK_LOCAL_INSTANCES_MASK = (1 << 12),
    LIGHTMAP_BLACK_NODE_MASK            = (1 << 13),

    LIGHTMAP_DEPTH                      = LIGHTMAP_ANY_MASK | LIGHTMAP_DEPTH_MASK | LIGHTMAP_IS_DYNAMIC_MASK | LIGHTMAP_EMITS_COLORS_MASK,
    LIGHTMAP_GLOBAL_STATIC              = LIGHTMAP_ANY_MASK | LIGHTMAP_COLOR_MASK | LIGHTMAP_IS_STATIC_MASK  | LIGHTMAP_EMITS_COLORS_MASK | LIGHTMAP_GLOBAL_STATIC_MASK,
    LIGHTMAP_GLOBAL_DYNAMIC             = LIGHTMAP_ANY_MASK | LIGHTMAP_COLOR_MASK | LIGHTMAP_IS_DYNAMIC_MASK | LIGHTMAP_EMITS_COLORS_MASK | LIGHTMAP_GLOBAL_DYNAMIC_MASK,
    LIGHTMAP_BLACK_GLOBAL_STATIC        = LIGHTMAP_ANY_MASK | LIGHTMAP_COLOR_MASK | LIGHTMAP_IS_STATIC_MASK  | LIGHTMAP_IS_BLACK_MASK | LIGHTMAP_BLACK_GLOBAL_STATIC_MASK,
    LIGHTMAP_BLACK_LOCAL_INSTANCES      = LIGHTMAP_ANY_MASK | LIGHTMAP_COLOR_MASK | LIGHTMAP_IS_DYNAMIC_MASK | LIGHTMAP_IS_BLACK_MASK | LIGHTMAP_BLACK_LOCAL_INSTANCES_MASK,
    LIGHTMAP_BLACK_NODE                 = LIGHTMAP_ANY_MASK | LIGHTMAP_COLOR_MASK | LIGHTMAP_IS_DYNAMIC_MASK | LIGHTMAP_IS_BLACK_MASK | LIGHTMAP_BLACK_NODE_MASK,
    
    LIGHTMAP_BLACK_GLOBAL_AND_LOCAL     = LIGHTMAP_BLACK_GLOBAL_STATIC | LIGHTMAP_BLACK_LOCAL_INSTANCES
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
