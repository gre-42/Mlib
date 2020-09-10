#pragma once

namespace Mlib {

class ShockAbsorber {
public:
    enum class IntegrationMode {
        IMPLICIT,
        EXPLICIT,
        SEMI_IMPLICIT
    };
    ShockAbsorber(float Ks, float Ka);
    void integrate_force(float f);
    void advance_time(float dt, IntegrationMode integration_mode = IntegrationMode::SEMI_IMPLICIT);
    inline float position() const {
        return position_;
    }
private:
    float Ks_;
    float Ka_;
    float position_;
    float F_;
};

}
