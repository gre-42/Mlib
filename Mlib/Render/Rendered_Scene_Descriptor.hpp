#pragma once
#include <Mlib/Scene_Graph/Render_Pass.hpp>

namespace Mlib {

struct RenderedSceneDescriptor {
    const ExternalRenderPass external_render_pass;
    const size_t time_id;
    const std::string light_node_name;
    std::strong_ordering operator <=> (const RenderedSceneDescriptor&) const = default;
};

}
