#pragma once
#include <Mlib/Array/Fixed_Array.hpp>

namespace Mlib::Sfm {

enum class TrackingMode;

class FlowingParticlesConfig {
public:
    TrackingMode tracking_mode;
    bool draw_optical_flow = false;
    size_t target_nparticles = 100;
    float presmoothing_sigma = 0.5f;
    float distance_sigma = 2.f;
    float worst_patch_error = 0.4f; // Errors respect the patch-brightness.
    FixedArray<size_t, 2> search_window{ 10u, 10u };
    FixedArray<size_t, 2> patch_size{ 10u, 10u };
};

}
