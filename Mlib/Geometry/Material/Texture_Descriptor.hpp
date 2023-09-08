#pragma once
#include <Mlib/Geometry/Material/Color_Mode.hpp>
#include <Mlib/Geometry/Material/Mipmap_Mode.hpp>
#include <Mlib/Geometry/Material/Wrap_Mode.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <iosfwd>
#include <string>

namespace Mlib {

struct TextureDescriptor {
    std::string color;
    std::string alpha;
    std::string specular;
    std::string normal;
    ColorMode color_mode = ColorMode::UNDEFINED;
    float alpha_fac = 1.f;
    bool desaturate = false;
    std::string histogram = "";
    std::string mixed = "";
    size_t overlap_npixels = 5;
    OrderableFixedArray<float, 3> mean_color = {-1.f, -1.f, -1.f};
    OrderableFixedArray<float, 3> lighten = {0.f, 0.f, 0.f};
    OrderableFixedArray<float, 3> lighten_top = {0.f, 0.f, 0.f};
    OrderableFixedArray<float, 3> lighten_bottom = {0.f, 0.f, 0.f};
    MipmapMode mipmap_mode = MipmapMode::NO_MIPMAPS;
    unsigned int anisotropic_filtering_level = 0;
    WrapMode wrap_mode_s = WrapMode::REPEAT;
    WrapMode wrap_mode_t = WrapMode::REPEAT;
    std::partial_ordering operator <=> (const TextureDescriptor&) const = default;
    template <class Archive>
    void serialize(Archive& archive) {
        archive(color);
        archive(alpha);
        archive(specular);
        archive(normal);
        archive(color_mode);
        archive(alpha_fac);
        archive(desaturate);
        archive(histogram);
        archive(mixed);
        archive(overlap_npixels);
        archive(mean_color);
        archive(lighten);
        archive(lighten_top);
        archive(lighten_bottom);
        archive(mipmap_mode);
        archive(anisotropic_filtering_level);
        archive(wrap_mode_s);
        archive(wrap_mode_t);
    }
};

inline std::ostream& operator << (std::ostream& ostr, const TextureDescriptor& t) {
    ostr <<
        "color: " << t.color << '\n' <<
        "alpha: " << t.alpha << '\n' <<
        "specular: " << t.specular << '\n' <<
        "normal: " << t.normal << '\n' <<
        "color_mode: " << color_mode_to_string(t.color_mode) << '\n' <<
        "desaturate: " << (int)t.desaturate << '\n' <<
        "histogram: " << t.histogram << '\n' <<
        "mixed: " << t.mixed << '\n' <<
        "overlap_npixels: " << t.overlap_npixels << '\n' <<
        "mean_color: " << t.mean_color << '\n' <<
        "lighten: " << t.lighten << '\n' <<
        "lighten_top: " << t.lighten_top << '\n' <<
        "lighten_bottom: " << t.lighten_bottom << '\n' <<
        "mipmap_mode: " << mipmap_mode_to_string(t.mipmap_mode) << '\n' <<
        "anisotropic_filtering_level: " << t.anisotropic_filtering_level << '\n';
    return ostr;
}

}
