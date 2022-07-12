#pragma once
#include <Mlib/Array/Array.hpp>
#include <Mlib/Sfm/Draw/Projector_With_Cameras.hpp>
#include <Mlib/Sfm/Sparse_Bundle/Marginalized_Map.hpp>

namespace Mlib {

template <class TDir, class TPos, size_t n>
class TransformationMatrix;

namespace Sfm {

class DenseProjector: public ProjectorWithCameras {

public:
    DenseProjector(
        const MarginalizedMap<std::map<std::chrono::milliseconds, CameraFrame>>& camera_frames,
        size_t i0,
        size_t i1,
        size_t iz,
        const Array<FixedArray<float, 3>>& x,
        const Array<float>& condition_number,
        const TransformationMatrix<float, float, 2>& ki,
        const TransformationMatrix<float, float, 3>& ke,
        const Array<float>& rgb);

    static DenseProjector from_image(
        const MarginalizedMap<std::map<std::chrono::milliseconds, CameraFrame>>& camera_frames,
        size_t i0,
        size_t i1,
        size_t iz,
        const Array<float>& x,
        const Array<float>& condition_number,
        const TransformationMatrix<float, float, 2>& ki,
        const TransformationMatrix<float, float, 3>& ke,
        const Array<float>& rgb);

    template <class TOperation, class TElse>
    void for_each_point(
        const TOperation& op,
        const TElse& else_);

    DenseProjector& normalize(float scale);

    void draw(const std::string& filename);

private:
    const Array<FixedArray<float, 3>> x_;
    const Array<FixedArray<float, 2>> y_;
    const Array<float> condition_number_;
    const Array<float> rgb_;
};

}}
