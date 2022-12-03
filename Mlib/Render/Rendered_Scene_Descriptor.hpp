#pragma once
#include <Mlib/Scene_Graph/Render_Pass.hpp>

namespace Mlib {

struct RenderedSceneDescriptor {
    ExternalRenderPass external_render_pass = {.pass = ExternalRenderPassType::STANDARD};
    size_t time_id = 0;
    std::string light_resource_suffix;
    std::strong_ordering operator <=> (const RenderedSceneDescriptor&) const = default;
};

class RootRenderedSceneDescriptor {
public:
    inline const RenderedSceneDescriptor& next(bool motion_interpolation) {
        if (motion_interpolation) {
            rsd_.time_id = (rsd_.time_id + 1) % 4;
        }
        return rsd_;
    }
private:
    RenderedSceneDescriptor rsd_;
};

}
