#pragma once

namespace Mlib {

struct PenetrationLimits {
    constexpr static const float FAC_R = 1 / 4.f;
    constexpr static const float FAC_T = 1 - FAC_R;
    constexpr static const float FAC_PAIR = 1 / 2.f;
    PenetrationLimits(float dt, float max_penetration)
    {
        float vmax_total = max_penetration / dt;
        vmax_rotation = vmax_total * FAC_R * FAC_PAIR;
        vmax_translation = vmax_total * FAC_T * FAC_PAIR;
    }
    inline float wmax(float radius) const {
        return vmax_rotation / radius;
    }
    float vmax_rotation;
    float vmax_translation;
};

}
