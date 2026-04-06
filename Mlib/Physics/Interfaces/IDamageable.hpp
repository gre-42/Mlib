#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>

namespace Mlib {

enum class DamageSource;

class IDamageable: public virtual DanglingBaseClass {
public:
    virtual float health() const = 0;
    virtual void damage(float amount, DamageSource source) = 0;
};

}
