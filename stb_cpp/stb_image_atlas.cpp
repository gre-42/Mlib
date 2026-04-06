#include "stb_image_atlas.hpp"
#include "stb_image_load.hpp"
#include <algorithm>
#include <stdexcept>

void build_image_atlas(
    const std::vector<std::shared_ptr<StbInfo<uint8_t>>>& atlas,
    const std::vector<AtlasTile>& tiles)
{
    for (auto& layer : atlas) {
        std::fill(
            layer->data(),
            layer->data() + layer->width * layer->height * layer->nrChannels,
            0);
    }
    for (const auto& [source, target] : tiles) {
        if (target.layer >= atlas.size()) {
            throw std::runtime_error("Tile layer out of bounds");
        }
        auto& atlas_layer = *atlas[target.layer];
        int width = std::min(source.image.width, source.width);
        int height = std::min(source.image.height, source.height);
        int nrChannels = std::min(source.image.nrChannels, atlas_layer.nrChannels);
        for (int r = 0; r < height; ++r) {
            for (int c = 0; c < width; ++c) {
                for (int d = 0; d < nrChannels; ++d) {
                    int rs = r + source.bottom;
                    int cs = c + source.left;
                    if ((rs < 0) || (rs >= source.image.height)) {
                        continue;
                    }
                    if ((cs < 0) || (cs >= source.image.width)) {
                        continue;
                    }
                    int rt = target.bottom + r;
                    int ct = target.left + c;
                    if ((rt < 0) || (rt >= atlas_layer.height)) {
                        continue;
                    }
                    if ((ct < 0) || (ct >= atlas_layer.width)) {
                        continue;
                    }
                    atlas_layer.data()[(rt * atlas_layer.width + ct) * atlas_layer.nrChannels + d] =
                        source.image.data()[(rs * source.image.width + cs) * source.image.nrChannels + d];
                }
            }
        }
    }
}
