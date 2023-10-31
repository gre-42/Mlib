#include "Render_Texture_Atlas.hpp"
#include <Mlib/Geometry/Material/Color_Mode.hpp>
#include <Mlib/Render/Render_Logics/Fill_With_Texture_Logic.hpp>
#include <Mlib/Render/Render_Logics/Resource_Update_Cycle.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Render/Viewport_Guard.hpp>

using namespace Mlib;

void Mlib::render_texture_atlas(
    RenderingResources& rendering_resources,
    const std::vector<AutoAtlasTileDescriptor>& tiles,
    float scale_width,
    float scale_height)
{
    for (const auto& tile : tiles) {
        ViewportGuard vg{
            (float)tile.left * scale_width,
            (float)tile.bottom * scale_height,
            (float)tile.width * scale_width,
            (float)tile.height * scale_height};
        FillWithTextureLogic logic{
            rendering_resources,
            tile.filename,
            ResourceUpdateCycle::ONCE,
            ColorMode::RGBA};
        logic.render();
    }
}
