#pragma once
#include <Mlib/Array/Array.hpp>
#include <Mlib/Images/Bgr24Raw.hpp>
#include <Mlib/Sfm/Draw/Projector_With_Cameras.hpp>
#include <Mlib/Sfm/Sparse_Bundle/Marginalized_Map.hpp>

namespace Mlib { namespace Sfm {

class DenseProjector: public ProjectorWithCameras {

public:
    DenseProjector(
        const MarginalizedMap<std::map<std::chrono::milliseconds, CameraFrame>>& camera_frames,
        size_t i0,
        size_t i1,
        size_t iz,
        const Array<float>& x,
        const Array<float>& condition_number,
        const Array<float>& ki,
        const Array<float>& ke,
        const Array<float>& rgb);

    static DenseProjector from_image(
        const MarginalizedMap<std::map<std::chrono::milliseconds, CameraFrame>>& camera_frames,
        size_t i0,
        size_t i1,
        size_t iz,
        const Array<float>& x,
        const Array<float>& condition_number,
        const Array<float>& ki,
        const Array<float>& ke,
        const Array<float>& rgb);

    template <class TOperation, class TElse>
    void for_each_point(
        const TOperation& op,
        const TElse& else_);

    DenseProjector& normalize(float scale);

    void draw(const std::string& filename);

private:
    const Array<float> x_;
    const Array<float> y_;
    const Array<float> condition_number_;
    const Array<float> rgb_;
};

}}
