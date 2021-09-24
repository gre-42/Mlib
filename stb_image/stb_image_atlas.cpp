#include "stb_image_atlas.hpp"
#include "stb_image_load.hpp"

void build_image_atlas(StbInfo& atlas, const std::vector<AtlasTile>& tiles) {
    std::fill(
        atlas.data.get(),
        atlas.data.get() + atlas.width * atlas.height * atlas.nrChannels,
        0);
    for (const AtlasTile& tile : tiles) {
        int nrChannels = std::min(tile.image.nrChannels, atlas.nrChannels);
        for (int r = 0; r < tile.image.height; ++r) {
            for (int c = 0; c < tile.image.width; ++c) {
                for (int d = 0; d < nrChannels; ++d) {
                    int ra = tile.bottom + r;
                    int ca = tile.left + c;
                    if ((ra < 0) || (ra >= atlas.height)) {
                        continue;
                    }
                    if ((ca < 0) || (ca >= atlas.width)) {
                        continue;
                    }
                    atlas.data.get()[(ra * atlas.width + ca) * atlas.nrChannels + d] =
                        tile.image.data.get()[(r * tile.image.width + c) * tile.image.nrChannels + d];
                }
            }
        }
    }
}
