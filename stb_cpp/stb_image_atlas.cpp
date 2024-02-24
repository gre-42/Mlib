#include "stb_image_atlas.hpp"
#include "stb_image_load.hpp"
#include <Mlib/Throw_Or_Abort.hpp>

void build_image_atlas(
    const std::vector<StbInfo<uint8_t>>& atlas,
    const std::vector<AtlasTile>& tiles)
{
    for (auto& layer : atlas) {
        std::fill(
            layer.data.get(),
            layer.data.get() + layer.width * layer.height * layer.nrChannels,
            0);
    }
    for (const AtlasTile& tile : tiles) {
        if (tile.layer >= atlas.size()) {
            THROW_OR_ABORT("Tile layer out of bounds");
        }
        auto& atlas_layer = atlas[tile.layer];
        int nrChannels = std::min(tile.image.nrChannels, atlas_layer.nrChannels);
        for (int r = 0; r < tile.image.height; ++r) {
            for (int c = 0; c < tile.image.width; ++c) {
                for (int d = 0; d < nrChannels; ++d) {
                    int ra = tile.bottom + r;
                    int ca = tile.left + c;
                    if ((ra < 0) || (ra >= atlas_layer.height)) {
                        continue;
                    }
                    if ((ca < 0) || (ca >= atlas_layer.width)) {
                        continue;
                    }
                    atlas_layer.data.get()[(ra * atlas_layer.width + ca) * atlas_layer.nrChannels + d] =
                        tile.image.data.get()[(r * tile.image.width + c) * tile.image.nrChannels + d];
                }
            }
        }
    }
}
