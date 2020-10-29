#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Physics/External_Force_Provider.hpp>
#include <memory>

namespace Mlib {

class RigidBody;

class GravityEfp: public ExternalForceProvider {
public:
    explicit GravityEfp(const FixedArray<float, 3>& gravity);
    virtual void increment_external_forces(const std::list<std::shared_ptr<RigidBody>>& olist, bool burn_in) override;
private:
    FixedArray<float, 3> gravity_;
};

}
