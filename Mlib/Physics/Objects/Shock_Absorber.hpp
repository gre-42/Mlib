#pragma once

namespace Mlib {

class ShockAbsorber {
public:
    ShockAbsorber(float Ks, float Ka);
    void integrate_force(float f);
    void advance_time(float dt, bool implicit = true);
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
