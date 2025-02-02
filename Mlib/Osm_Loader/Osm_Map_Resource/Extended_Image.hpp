#pragma once
#include <Mlib/Array/Array.hpp>

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
    bool operator () (double r, double c, double& value) const;
    inline size_t original_shape(size_t i) const {
        return original_shape_(i);
    }
private:
    ArrayShape original_shape_;
    Array<double> extended_image_;
    double dextension_;
};

}
