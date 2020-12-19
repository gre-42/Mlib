#pragma once

namespace Mlib {

class Damageable {
public:
    virtual float health() const = 0;
    virtual void damage(float amount) = 0;
};

}
