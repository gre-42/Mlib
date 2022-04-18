#include "Render_Pass.hpp"
#include <stdexcept>

using namespace Mlib;

ExternalRenderPassType Mlib::external_render_pass_type_from_string(const std::string& str) {
    if (str == "standard") {
        return ExternalRenderPassType::STANDARD;
    } else if (str == "lightmap_global_static") {
        return ExternalRenderPassType::LIGHTMAP_GLOBAL_STATIC;
    } else if (str == "lightmap_global_dynamic") {
        return ExternalRenderPassType::LIGHTMAP_GLOBAL_DYNAMIC;
    } else if (str == "lightmap_black_global_static") {
        return ExternalRenderPassType::LIGHTMAP_BLACK_GLOBAL_STATIC;
    } else if (str == "lightmap_black_local_instances") {
        return ExternalRenderPassType::LIGHTMAP_BLACK_LOCAL_INSTANCES;
    } else if (str == "lightmap_black_node") {
        return ExternalRenderPassType::LIGHTMAP_BLACK_NODE;
    } else if (str == "dirtmap") {
        return ExternalRenderPassType::DIRTMAP;
    } else {
        throw std::runtime_error("Unknown render pass type: \"" + str + '"');
    }
}

std::string Mlib::external_render_pass_type_to_string(ExternalRenderPassType pass) {
    switch(pass) {
        case ExternalRenderPassType::STANDARD: return "standard";
        case ExternalRenderPassType::LIGHTMAP_GLOBAL_STATIC: return "lightmap_global_static";
        case ExternalRenderPassType::LIGHTMAP_GLOBAL_DYNAMIC: return "lightmap_global_dynamic";
        case ExternalRenderPassType::LIGHTMAP_BLACK_GLOBAL_STATIC: return "lightmap_black_global_static";
        case ExternalRenderPassType::LIGHTMAP_BLACK_LOCAL_INSTANCES: return "lightmap_black_local_instances";
        case ExternalRenderPassType::LIGHTMAP_BLACK_NODE: return "lightmap_black_node";
        case ExternalRenderPassType::DIRTMAP: return "dirtmap";
        default:
            throw std::runtime_error("Unknown render pass type");
    }
}
