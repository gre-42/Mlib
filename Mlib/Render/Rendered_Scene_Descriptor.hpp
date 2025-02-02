#pragma once
#include <Mlib/Geometry/Material/Render_Pass.hpp>
#include <Mlib/Scene_Graph/Render_Pass_Extended.hpp>

namespace Mlib {

struct RenderedSceneDescriptor {
    ExternalRenderPass external_render_pass = {.pass = ExternalRenderPassType::STANDARD};
    size_t time_id = 0;
    std::strong_ordering operator <=> (const RenderedSceneDescriptor&) const = default;
};

class RootRenderedSceneDescriptor {
public:
    inline const RenderedSceneDescriptor& next(
            bool motion_interpolation,
            std::chrono::steady_clock::time_point time)
    {
        if (motion_interpolation) {
            rsd_.time_id = (rsd_.time_id + 1) % 4;
        }
        rsd_.external_render_pass.time = time;
        return rsd_;
    }
private:
    RenderedSceneDescriptor rsd_;
};

}
