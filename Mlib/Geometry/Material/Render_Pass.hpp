#pragma once
#include <iosfwd>
#include <string>

namespace Mlib {

/**
 * Ordering is only important for the occluded-pass, not the occluder-pass.
 * See Mlib/Render/Renderables/Renderable_Colored_Vertex_Array.cpp:
 *     "cva->material.occluded_pass < l.second->shadow_render_pass"
 */
enum class ExternalRenderPassType {
    NONE                                    = 0,
    STANDARD_MASK                           = (1 << 0),

    DIRTMAP_MASK                            = (1 << 1),

    IS_GLOBAL_MASK                          = (1 << 2),
    IS_STATIC_MASK                          = (1 << 3),

    LIGHTMAP_ANY_MASK                       = (1 << 4),
    LIGHTMAP_BLOBS_MASK                     = (1 << 5),
    LIGHTMAP_DEPTH_MASK                     = (1 << 6),
    LIGHTMAP_COLOR_MASK                     = (1 << 7),
    LIGHTMAP_EMITS_COLORS_MASK              = (1 << 8),
    LIGHTMAP_IS_BLACK_MASK                  = (1 << 9),
    LIGHTMAP_IS_DYNAMIC_MASK                = (1 << 10),
    LIGHTMAP_IS_LOCAL_MASK                  = (1 << 11),

    LIGHTMAP_GLOBAL_STATIC_MASK             = (1 << 12),
    LIGHTMAP_GLOBAL_DYNAMIC_MASK            = (1 << 13),
    LIGHTMAP_BLACK_GLOBAL_STATIC_MASK       = (1 << 14),
    LIGHTMAP_BLACK_LOCAL_INSTANCES_MASK     = (1 << 15),
    LIGHTMAP_BLACK_MOVABLES_MASK            = (1 << 16),
    LIGHTMAP_BLACK_NODE_MASK                = (1 << 17),

    IMPOSTER_NODE_MASK                      = (1 << 18),
    ZOOM_NODE_MASK                          = (1 << 19),

    STANDARD                                = STANDARD_MASK | IS_STATIC_MASK,

    DIRTMAP                                 = DIRTMAP_MASK | IS_GLOBAL_MASK,
    LIGHTMAP_BLOBS                          = LIGHTMAP_ANY_MASK | LIGHTMAP_COLOR_MASK | LIGHTMAP_IS_DYNAMIC_MASK | LIGHTMAP_EMITS_COLORS_MASK | LIGHTMAP_IS_LOCAL_MASK | LIGHTMAP_BLOBS_MASK,
    LIGHTMAP_DEPTH                          = LIGHTMAP_ANY_MASK | LIGHTMAP_DEPTH_MASK | LIGHTMAP_IS_DYNAMIC_MASK | LIGHTMAP_EMITS_COLORS_MASK,
    LIGHTMAP_GLOBAL_STATIC                  = LIGHTMAP_ANY_MASK | LIGHTMAP_COLOR_MASK | IS_GLOBAL_MASK           | LIGHTMAP_EMITS_COLORS_MASK | LIGHTMAP_GLOBAL_STATIC_MASK,
    LIGHTMAP_GLOBAL_DYNAMIC                 = LIGHTMAP_ANY_MASK | LIGHTMAP_COLOR_MASK | LIGHTMAP_IS_DYNAMIC_MASK | LIGHTMAP_EMITS_COLORS_MASK | LIGHTMAP_GLOBAL_DYNAMIC_MASK,
    LIGHTMAP_BLACK_GLOBAL_STATIC            = LIGHTMAP_ANY_MASK | LIGHTMAP_COLOR_MASK | IS_GLOBAL_MASK           | LIGHTMAP_IS_BLACK_MASK | LIGHTMAP_BLACK_GLOBAL_STATIC_MASK,
    LIGHTMAP_BLACK_LOCAL_INSTANCES          = LIGHTMAP_ANY_MASK | LIGHTMAP_COLOR_MASK | LIGHTMAP_IS_DYNAMIC_MASK | LIGHTMAP_IS_BLACK_MASK | LIGHTMAP_IS_LOCAL_MASK | LIGHTMAP_BLACK_LOCAL_INSTANCES_MASK,
    LIGHTMAP_BLACK_MOVABLES                 = LIGHTMAP_ANY_MASK | LIGHTMAP_COLOR_MASK | LIGHTMAP_IS_DYNAMIC_MASK | LIGHTMAP_IS_BLACK_MASK | LIGHTMAP_BLACK_MOVABLES_MASK,
    LIGHTMAP_BLACK_NODE                     = LIGHTMAP_ANY_MASK | LIGHTMAP_COLOR_MASK | LIGHTMAP_IS_DYNAMIC_MASK | LIGHTMAP_IS_BLACK_MASK | LIGHTMAP_BLACK_NODE_MASK,

    LIGHTMAP_BLACK_GLOBAL_AND_LOCAL         = LIGHTMAP_BLACK_GLOBAL_STATIC | LIGHTMAP_BLACK_LOCAL_INSTANCES,

    IMPOSTER_NODE                           = IMPOSTER_NODE_MASK,
    ZOOM_NODE                               = ZOOM_NODE_MASK,

    STANDARD_OR_IMPOSTER_OR_ZOOM_NODE       = STANDARD_MASK | IMPOSTER_NODE_MASK | ZOOM_NODE_MASK,
    IMPOSTER_OR_ZOOM_NODE                   = IMPOSTER_NODE_MASK | ZOOM_NODE_MASK
};

inline ExternalRenderPassType operator & (ExternalRenderPassType a, ExternalRenderPassType b) {
    return (ExternalRenderPassType)((int)a & (int)b);
}

inline ExternalRenderPassType operator | (ExternalRenderPassType a, ExternalRenderPassType b) {
    return (ExternalRenderPassType)((int)a | (int)b);
}

inline bool any(ExternalRenderPassType v) {
    return v != ExternalRenderPassType::NONE;
}

ExternalRenderPassType external_render_pass_type_from_string(const std::string& str);
std::string external_render_pass_type_to_string(ExternalRenderPassType pass);

std::ostream& operator << (std::ostream& ostr, ExternalRenderPassType pass);

}
