#pragma once

namespace Mlib {

class IDamageable {
public:
    virtual float health() const = 0;
    virtual void damage(float amount) = 0;
};

}
