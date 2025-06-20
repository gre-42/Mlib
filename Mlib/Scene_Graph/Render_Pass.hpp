#pragma once
#include <Mlib/Scene_Graph/Render_Passes.hpp>
#include <Mlib/Scene_Graph/Rendered_Scene_Descriptor.hpp>

namespace Mlib {

struct RenderPass {
    const RenderedSceneDescriptor rsd;
    const InternalRenderPass internal;
};

}
