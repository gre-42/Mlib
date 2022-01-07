#pragma once
#include <Mlib/Scene_Graph/Render_Pass.hpp>

namespace Mlib {

class SceneNode;

struct RenderedSceneDescriptor {
    const ExternalRenderPass external_render_pass = {.pass = ExternalRenderPassType::STANDARD_WITH_POSTPROCESSING, .black_node_name = ""};
    const size_t time_id = 0;
    const std::string light_node_name = "";
    const SceneNode* camera_node = nullptr;
    std::strong_ordering operator <=> (const RenderedSceneDescriptor&) const = default;
};

}
