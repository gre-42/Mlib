#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Scene_Graph/Camera.hpp>

namespace Mlib {

class ProjectionMatrixCamera: public Camera {
public:
    explicit ProjectionMatrixCamera(const FixedArray<float, 4, 4>& projection_matrix);
    virtual ~ProjectionMatrixCamera() override;
    virtual std::shared_ptr<Camera> copy() const override;
    virtual FixedArray<float, 4, 4> projection_matrix() override;
private:
    FixedArray<float, 4, 4> projection_matrix_;
};

}
