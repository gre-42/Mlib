#pragma once
#include <Mlib/Geometry/Material/Color_Mode.hpp>
#include <Mlib/Geometry/Material/Colormap_With_Modifiers.hpp>
#include <Mlib/Geometry/Material/Mipmap_Mode.hpp>
#include <Mlib/Geometry/Material/Normalmap_With_Modifiers.hpp>
#include <Mlib/Geometry/Material/Wrap_Mode.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <iosfwd>
#include <string>

namespace Mlib {

struct TextureDescriptor {
    ColormapWithModifiers color;
    std::string specular;
    NormalmapWithModifiers normal;
    ColorMode color_mode = ColorMode::UNDEFINED;
    float alpha_fac = 1.f;
    MipmapMode mipmap_mode = MipmapMode::NO_MIPMAPS;
    unsigned int anisotropic_filtering_level = 0;
    OrderableFixedArray<WrapMode, 2> wrap_modes = {WrapMode::REPEAT, WrapMode::REPEAT};
    std::partial_ordering operator <=> (const TextureDescriptor&) const = default;
    template <class Archive>
    void serialize(Archive& archive) {
        archive(color);
        archive(specular);
        archive(normal);
        archive(color_mode);
        archive(alpha_fac);
        archive(mipmap_mode);
        archive(anisotropic_filtering_level);
        archive(wrap_modes);
    }
};

std::ostream& operator << (std::ostream& ostr, const TextureDescriptor& t);

}
