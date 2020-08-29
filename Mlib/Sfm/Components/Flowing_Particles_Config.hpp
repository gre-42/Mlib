#pragma once
#include <Mlib/Array/Array_Shape.hpp>

namespace Mlib { namespace Sfm {

enum class TrackingMode {
    PATCH_NEW_POSITION_IN_BOX
};

class FlowingParticlesConfig {
public:
    TrackingMode tracking_mode = TrackingMode::PATCH_NEW_POSITION_IN_BOX;
    bool draw_optical_flow = false;
    size_t target_nparticles = 50;
    float distance_sigma = 2;
    float worst_patch_error = 0.4f; // Errors respect the patch-brightness.
    ArrayShape search_window{10, 10};
    ArrayShape patch_size{10, 10};
};

}}
