#pragma once
#include <Mlib/Array/Array.hpp>
#include <Mlib/Scene_Precision.hpp>

namespace Mlib {

class ExtendedImage {
public:
    ExtendedImage(
        const Array<double>& image,
        const Array<bool>& mask,
        size_t extension,
        size_t box_filter_radius,
        size_t niterations,
        bool preserve_original = true);
    bool operator () (CompressedScenePos r, CompressedScenePos c, CompressedScenePos& value) const;
    inline size_t original_shape(size_t i) const {
        return extended_image_.shape(i);
    }
private:
    Array<double> extended_image_;
    size_t extension_;
};

}
