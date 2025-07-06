#pragma once

namespace Mlib {

struct PenetrationLimits;

class PenetrationLimitsFactory {
public:
    PenetrationLimitsFactory(float max_penetration, float radius);
    static PenetrationLimitsFactory inf();
    float vmax_translation(float dt) const;
    float wmax(float dt) const;
private:
    float max_penetration_;
    float radius_;
};

}
