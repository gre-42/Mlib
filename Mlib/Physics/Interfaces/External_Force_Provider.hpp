#pragma once
#include <list>
#include <memory>

namespace Mlib {

class RigidBody;

class ExternalForceProvider {
public:
    virtual void increment_external_forces(const std::list<std::shared_ptr<RigidBody>>& olist, bool burn_in) = 0;
};

}
