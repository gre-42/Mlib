#pragma once
#include <Mlib/Scene_Graph/Render_Pass.hpp>

namespace Mlib {

struct RenderedSceneDescriptor {
    ExternalRenderPass external_render_pass = {.pass = ExternalRenderPassType::STANDARD};
    size_t time_id = 0;
    std::string light_resource_suffix;
    std::strong_ordering operator <=> (const RenderedSceneDescriptor&) const = default;
};

}
