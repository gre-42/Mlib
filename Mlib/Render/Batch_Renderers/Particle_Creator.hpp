#pragma once
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/Billboard_Sequence.hpp>
#include <Mlib/Scene_Graph/Interfaces/IParticle_Creator.hpp>

namespace Mlib {

class ParticlesInstance;
struct BillboardSequence;

class ParticleCreator final: public IParticleCreator {
public:
    ParticleCreator(
        ParticlesInstance &particles_instance,
        const BillboardSequence& billboard_sequence);
    
    ~ParticleCreator();

    virtual void add_particle(
        const TransformationMatrix<float, ScenePos, 3>& transformation_matrix,
        const FixedArray<float, 3>& velocity,
        float air_resistance) override;
private:
    ParticlesInstance &particles_instance_;
    BillboardSequence billboard_sequence_;
};

}
