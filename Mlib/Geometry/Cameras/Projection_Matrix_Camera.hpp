#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Cameras/Camera.hpp>

namespace Mlib {

class ProjectionMatrixCamera: public Camera {
public:
    explicit ProjectionMatrixCamera(const FixedArray<float, 4, 4>& projection_matrix);
    virtual ~ProjectionMatrixCamera() override;
    virtual std::unique_ptr<Camera> copy() const override;
    virtual FixedArray<float, 4, 4> projection_matrix() const override;
    virtual FixedArray<float, 2> dpi(const FixedArray<float, 2>& texture_size) const override;
private:
    FixedArray<float, 4, 4> projection_matrix_;
};

}
