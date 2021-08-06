#pragma once
#include <Mlib/Array/Array_Shape.hpp>

namespace Mlib::Sfm {

enum class TrackingMode {
    PATCH_NEW_POSITION_IN_BOX,
    SIFT
};

class FlowingParticlesConfig {
public:
    TrackingMode tracking_mode = TrackingMode::PATCH_NEW_POSITION_IN_BOX;
    bool draw_optical_flow = false;
    size_t target_nparticles = 100;
    float presmoothing_sigma = 0.5f;
    float distance_sigma = 2.f;
    float worst_patch_error = 0.4f; // Errors respect the patch-brightness.
    FixedArray<size_t, 2> search_window{10, 10};
    FixedArray<size_t, 2> patch_size{10, 10};
};

}
