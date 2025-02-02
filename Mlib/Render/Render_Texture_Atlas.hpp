#pragma once
#include <vector>

namespace Mlib {

struct AutoAtlasTileDescriptor;
class RenderingResources;

void render_texture_atlas(
    const RenderingResources& rendering_resources,
    const std::vector<AutoAtlasTileDescriptor>& tiles,
    float scale_width,
    float scale_height);

}
