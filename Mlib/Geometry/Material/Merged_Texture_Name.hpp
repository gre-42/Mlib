#pragma once

namespace Mlib {

struct Material;
struct ColormapWithModifiers;

struct MergedTextureName {
    explicit MergedTextureName(const Material& material);
    const ColormapWithModifiers& colormap;
};

}
