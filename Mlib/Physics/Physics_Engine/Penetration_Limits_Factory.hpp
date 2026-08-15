#pragma once

namespace Mlib {

enum class LimitSources;

class PenetrationLimitsFactory {
public:
    PenetrationLimitsFactory(
        float max_penetration,
        float radius,
        LimitSources limit_sources);
    static PenetrationLimitsFactory inf();
    float vmax_translation(float dt) const;
    float wmax(float dt) const;
private:
    float max_penetration_;
    float radius_;
    LimitSources limit_sources_;
};

}
