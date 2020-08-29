#pragma once
#include <Mlib/Geometry/Base_Projector.hpp>
#include <Mlib/Images/Image_fwd.hpp>
#include <Mlib/Sfm/Frames/Camera_Frame.hpp>
#include <Mlib/Sfm/Sparse_Bundle/Marginalized_Map.hpp>
#include <chrono>
#include <map>

namespace Mlib { namespace Sfm {

class ProjectorWithCameras: public BaseProjector {
public:
    ProjectorWithCameras(
        const MarginalizedMap<std::map<std::chrono::milliseconds, CameraFrame>>& camera_frames,
        size_t i0,
        size_t i1,
        size_t iz,
        const Array<float>& scale_matrix = identity_scale_matrix());

protected:
    void plot_camera_lines(PpmImage& ppm);
    void plot_camera_positions(PpmImage& ppm);
    void plot_unit_square(PpmImage& ppm);
    const MarginalizedMap<std::map<std::chrono::milliseconds, CameraFrame>>& camera_frames_;
};

}}
