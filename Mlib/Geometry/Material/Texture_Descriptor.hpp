#pragma once
#include <Mlib/Geometry/Material/Colormap_With_Modifiers.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <iosfwd>
#include <string>

namespace Mlib {

struct TextureDescriptor {
    ColormapWithModifiers color = ColormapWithModifiers{}.compute_hash();
    ColormapWithModifiers specular = ColormapWithModifiers{}.compute_hash();
    ColormapWithModifiers normal = ColormapWithModifiers{}.compute_hash();
    std::partial_ordering operator <=> (const TextureDescriptor&) const = default;
    template <class Archive>
    void serialize(Archive& archive) {
        archive(color);
        archive(specular);
        archive(normal);
    }
};

std::ostream& operator << (std::ostream& ostr, const TextureDescriptor& t);

}
