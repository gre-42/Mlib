#pragma once
#include <Mlib/Array/Array.hpp>

namespace Mlib { namespace Sfm {

class CameraFrame {
public:
    static const Array<float> undefined_kep;
    CameraFrame(
        const Array<float>& rotation,
        const Array<float>& position,
        const Array<float>& kep);
    Array<float> rotation;
    Array<float> position;
    Array<float> kep;
    Array<float> projection_matrix_3x4() const;
    Array<float> reconstruction_matrix_3x4() const;
    bool point_in_fov(const Array<float>& x, float threshold) const;
    Array<float> dir(size_t i) const;
    void set_from_projection_matrix_3x4(
        const Array<float>& projection,
        const Array<float>& kep);
private:
    void set_kep_if_undefined();
};

}}
