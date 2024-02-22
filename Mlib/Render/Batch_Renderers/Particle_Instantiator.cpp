#include "Particle_Instantiator.hpp"
#include <Mlib/Render/Batch_Renderers/Particles_Instance.hpp>

using namespace Mlib;

ParticleInstantiator::ParticleInstantiator(
    ParticlesInstance& particles_instance,
    const BillboardSequence& billboard_sequence)
    : particles_instance_{ particles_instance }
    , billboard_sequence_{ billboard_sequence }
{
    for (const auto& id : billboard_sequence.billboard_ids) {
        if (id >= particles_instance.num_billboard_atlas_components()) {
            THROW_OR_ABORT("Particle index out of bounds");
        }
    }
}

ParticleInstantiator::~ParticleInstantiator() = default;

void ParticleInstantiator::add_particle(const TransformationMatrix<float, double, 3>& transformation_matrix)
{
    particles_instance_.add_particle(transformation_matrix, billboard_sequence_);
}
