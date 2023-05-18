#pragma once
#include <Mlib/Scene_Graph/Interfaces/IParticle_Instantiator.hpp>

namespace Mlib {

class ParticlesInstance;
struct BillboardSequence;

class ParticleInstantiator final: public IParticleInstantiator {
public:
    ParticleInstantiator(
        ParticlesInstance &particles_instance,
        const BillboardSequence& billboard_sequence);
    
    ~ParticleInstantiator();

    virtual void add_particle(const TransformationMatrix<float, double, 3>& transformation_matrix) override;
private:
    ParticlesInstance &particles_instance_;
    const BillboardSequence& billboard_sequence_;
};

}
