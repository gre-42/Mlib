#include "Render_Pass.hpp"
#include <ostream>
#include <stdexcept>

using namespace Mlib;

ExternalRenderPassType Mlib::external_render_pass_type_from_string(const std::string& str) {
    if (str == "none") {
        return ExternalRenderPassType::NONE;
    } else if (str == "standard") {
        return ExternalRenderPassType::STANDARD;
    } else if (str == "dirtmap") {
        return ExternalRenderPassType::DIRTMAP;
    } else if (str == "lightmap_depth") {
        return ExternalRenderPassType::LIGHTMAP_DEPTH;
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
    } else if (str == "lightmap_black_global_and_local") {
        return ExternalRenderPassType::LIGHTMAP_BLACK_GLOBAL_AND_LOCAL;
    } else if (str == "lightmap_blobs") {
        return ExternalRenderPassType::LIGHTMAP_BLOBS;
    } else {
        throw std::runtime_error("Unknown render pass type: \"" + str + '"');
    }
}

std::string Mlib::external_render_pass_type_to_string(ExternalRenderPassType pass) {
    switch(pass) {
        case ExternalRenderPassType::STANDARD: return "standard";
        case ExternalRenderPassType::DIRTMAP: return "dirtmap";
        case ExternalRenderPassType::LIGHTMAP_DEPTH: return "lightmap_depth";
        case ExternalRenderPassType::LIGHTMAP_GLOBAL_STATIC: return "lightmap_global_static";
        case ExternalRenderPassType::LIGHTMAP_GLOBAL_DYNAMIC: return "lightmap_global_dynamic";
        case ExternalRenderPassType::LIGHTMAP_BLACK_GLOBAL_STATIC: return "lightmap_black_global_static";
        case ExternalRenderPassType::LIGHTMAP_BLACK_LOCAL_INSTANCES: return "lightmap_black_local_instances";
        case ExternalRenderPassType::LIGHTMAP_BLACK_NODE: return "lightmap_black_node";
        case ExternalRenderPassType::LIGHTMAP_BLACK_GLOBAL_AND_LOCAL: return "lightmap_black_global_and_local";
        case ExternalRenderPassType::LIGHTMAP_BLOBS: return "lightmap_blobs";
        default:
            throw std::runtime_error("Unknown render pass type");
    }
}

std::ostream& Mlib::operator << (std::ostream& ostr, ExternalRenderPassType pass) {
    ostr << external_render_pass_type_to_string(pass);
    return ostr;
}
