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
    } else if (str == "lightmap_local_instances_static") {
        return ExternalRenderPassType::LIGHTMAP_LOCAL_INSTANCES_STATIC;
    } else if (str == "lightmap_node_dynamic") {
        return ExternalRenderPassType::LIGHTMAP_NODE_DYNAMIC;
    } else if (str == "dirtmap") {
        return ExternalRenderPassType::DIRTMAP;
    } else {
        throw std::runtime_error("Unknown render pass type: \"" + str + '"');
    }
}
