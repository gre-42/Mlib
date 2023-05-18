#pragma once
#include <memory>

namespace Mlib {

class IParticlesInstance;

class IParticlesResource {
public:
    virtual ~IParticlesResource() = default;
    virtual std::unique_ptr<IParticlesInstance> instantiate_particles() = 0;
};

}
