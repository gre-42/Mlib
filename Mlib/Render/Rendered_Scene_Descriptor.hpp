#pragma once
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <Mlib/Scene_Graph/Render_Pass.hpp>
#include <tuple>

namespace Mlib {

struct RenderedSceneDescriptor {
    const ExternalRenderPass external_render_pass;
    const size_t time_id;
    const size_t light_resource_id;
    inline bool operator < (const RenderedSceneDescriptor& other) const {
        return std::make_tuple(
            external_render_pass,
            time_id,
            light_resource_id) <
            std::make_tuple(
                other.external_render_pass,
                other.time_id,
                other.light_resource_id);
    }
};

}
