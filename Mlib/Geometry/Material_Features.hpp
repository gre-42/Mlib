#pragma once
#include <cstddef>
#include <map>
#include <vector>

namespace Mlib {

struct BlendMapTexture;
struct ColormapWithModifiers;
struct Material;
template <typename TData, size_t... tshape>
class FixedArray;

struct BlendMapTextureAndId {
    const BlendMapTexture* ops;
    size_t id_color;
    size_t id_specular;
    size_t id_normal;
    const BlendMapTexture& operator * () const {
        return *ops;
    }
    const BlendMapTexture* operator -> () const {
        return ops;
    }
};

bool has_normalmap(const std::vector<BlendMapTexture>& textures_color);
bool fragments_depend_on_distance(
    const FixedArray<float, 2>& fog_distances,
    const FixedArray<float, 4>& alpha_distances,
    const std::vector<BlendMapTextureAndId>& textures,
	const std::map<ColormapWithModifiers, size_t>& texture_ids_color);
bool fragments_depend_on_normal(const std::vector<BlendMapTexture>& textures_color);
bool has_horizontal_detailmap(
    const std::vector<BlendMapTextureAndId>& textures,
    const std::map<ColormapWithModifiers, size_t>& texture_ids);

}
