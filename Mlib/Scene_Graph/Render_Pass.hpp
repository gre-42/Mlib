#pragma once
#include <string>

namespace Mlib {

struct ExternalRenderPass {
    const enum Pass {
        UNDEFINED,
        STANDARD_WITH_POSTPROCESSING,
        STANDARD_WO_POSTPROCESSING,
        LIGHTMAP_TO_TEXTURE,
        DIRTMAP
    } pass;
    const std::string black_node_name;
    inline bool operator < (const ExternalRenderPass& other) const {
        return std::make_pair(
            pass,
            black_node_name) <
            std::make_pair(
                other.pass,
                other.black_node_name);
    }
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
