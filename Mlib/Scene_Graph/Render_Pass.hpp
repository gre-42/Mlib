#pragma once
#include <string>

namespace Mlib {

enum class ExternalRenderPassType {
    STANDARD,
    LIGHTMAP_GLOBAL_STATIC,
    LIGHTMAP_GLOBAL_DYNAMIC,
    LIGHTMAP_BLACK_GLOBAL_STATIC,
    LIGHTMAP_BLACK_LOCAL_INSTANCES,
    LIGHTMAP_BLACK_NODE,
    DIRTMAP
};

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
