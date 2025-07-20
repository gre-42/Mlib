#include "Render_Pass.hpp"
#include <Mlib/Throw_Or_Abort.hpp>
#include <map>
#include <ostream>

using namespace Mlib;

ExternalRenderPassType Mlib::external_render_pass_type_from_string(const std::string& str) {
    static const std::map<std::string, ExternalRenderPassType> m{
        {"none", ExternalRenderPassType::NONE},
        {"standard", ExternalRenderPassType::STANDARD},
        {"dirtmap", ExternalRenderPassType::DIRTMAP},
        {"lightmap_depth", ExternalRenderPassType::LIGHTMAP_DEPTH},
        {"lightmap_global_static", ExternalRenderPassType::LIGHTMAP_GLOBAL_STATIC},
        {"lightmap_global_dynamic", ExternalRenderPassType::LIGHTMAP_GLOBAL_DYNAMIC},
        {"lightmap_black_global_static", ExternalRenderPassType::LIGHTMAP_BLACK_GLOBAL_STATIC},
        {"lightmap_black_local_instances", ExternalRenderPassType::LIGHTMAP_BLACK_LOCAL_INSTANCES},
        {"lightmap_black_movables", ExternalRenderPassType::LIGHTMAP_BLACK_MOVABLES},
        {"lightmap_black_node", ExternalRenderPassType::LIGHTMAP_BLACK_NODE},
        {"lightmap_black_global_and_local", ExternalRenderPassType::LIGHTMAP_BLACK_GLOBAL_AND_LOCAL},
        {"lightmap_blobs", ExternalRenderPassType::LIGHTMAP_BLOBS}
    };
    auto it = m.find(str);
    if (it == m.end()) {
        THROW_OR_ABORT("Unknown render pass type: \"" + str + '"');
    }
    return it->second;
}

std::string Mlib::external_render_pass_type_to_string(ExternalRenderPassType pass) {
    switch (pass) {
    case ExternalRenderPassType::STANDARD: return "standard";
    case ExternalRenderPassType::DIRTMAP: return "dirtmap";
    case ExternalRenderPassType::LIGHTMAP_DEPTH: return "lightmap_depth";
    case ExternalRenderPassType::LIGHTMAP_GLOBAL_STATIC: return "lightmap_global_static";
    case ExternalRenderPassType::LIGHTMAP_GLOBAL_DYNAMIC: return "lightmap_global_dynamic";
    case ExternalRenderPassType::LIGHTMAP_BLACK_GLOBAL_STATIC: return "lightmap_black_global_static";
    case ExternalRenderPassType::LIGHTMAP_BLACK_LOCAL_INSTANCES: return "lightmap_black_local_instances";
    case ExternalRenderPassType::LIGHTMAP_BLACK_MOVABLES: return "lightmap_black_movables";
    case ExternalRenderPassType::LIGHTMAP_BLACK_NODE: return "lightmap_black_node";
    case ExternalRenderPassType::LIGHTMAP_BLACK_GLOBAL_AND_LOCAL: return "lightmap_black_global_and_local";
    case ExternalRenderPassType::LIGHTMAP_BLOBS: return "lightmap_blobs";
    case ExternalRenderPassType::STANDARD_FOREGROUND: return "standard|foreground";
    case ExternalRenderPassType::STANDARD_BACKGROUND: return "standard|background";
    default:
        THROW_OR_ABORT("Unknown render pass type");
    }
}

std::ostream& Mlib::operator << (std::ostream& ostr, ExternalRenderPassType pass) {
    ostr << external_render_pass_type_to_string(pass);
    return ostr;
}
