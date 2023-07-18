#pragma once
#include <vector>

namespace Mlib {

class RenderingResources;
struct AutoAtlasTileDescriptor;

void render_texture_atlas(
    const std::vector<AutoAtlasTileDescriptor>& tiles,
    const RenderingResources& rendering_resources);

}
