#pragma once

namespace Mlib {

enum class DamageSource;

class IDamageable {
public:
    virtual float health() const = 0;
    virtual void damage(float amount, DamageSource source) = 0;
};

}
