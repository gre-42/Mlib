#pragma once
#include <Mlib/Geometry/Base_Projector.hpp>
#include <Mlib/Sfm/Frames/Camera_Frame.hpp>
#include <Mlib/Sfm/Sparse_Bundle/Marginalized_Map.hpp>
#include <chrono>
#include <map>

namespace Mlib {
   
class StbImage;

namespace Sfm {

class ProjectorWithCameras: public BaseProjector {
public:
    ProjectorWithCameras(
        const MarginalizedMap<std::map<std::chrono::milliseconds, CameraFrame>>& camera_frames,
        size_t i0,
        size_t i1,
        size_t iz,
        const FixedArray<float, 3, 3>& scale_matrix = identity_scale_matrix());

protected:
    void plot_camera_lines(StbImage& ppm);
    void plot_camera_positions(StbImage& ppm);
    void plot_unit_square(StbImage& ppm);
    const MarginalizedMap<std::map<std::chrono::milliseconds, CameraFrame>>& camera_frames_;
};

}}
