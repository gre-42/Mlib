#include "Render_Texture_Atlas.hpp"
#include <Mlib/Geometry/Material/Color_Mode.hpp>
#include <Mlib/Render/Render_Logics/Fill_With_Texture_Logic.hpp>
#include <Mlib/Render/Render_Logics/Resource_Update_Cycle.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Render/Viewport_Guard.hpp>

using namespace Mlib;

void Mlib::render_texture_atlas(
    const std::vector<AutoAtlasTileDescriptor>& tiles,
    const RenderingResources& rendering_resources)
{
    for (const auto& tile : tiles) {
        ViewportGuard vg{
            (float)tile.left,
            (float)tile.bottom,
            (float)tile.width,
            (float)tile.height};
        FillWithTextureLogic logic{tile.filename, ResourceUpdateCycle::ONCE, ColorMode::RGBA};
        logic.render();
    }
}
