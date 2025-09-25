#include "Particle_Creator.hpp"
#include <Mlib/Render/Batch_Renderers/Particles_Instance.hpp>

using namespace Mlib;

ParticleCreator::ParticleCreator(
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

ParticleCreator::~ParticleCreator() = default;

void ParticleCreator::add_particle(
    const TransformationMatrix<float, ScenePos, 3>& transformation_matrix,
    const FixedArray<float, 3>& velocity,
    float air_resistance,
    float texture_layer)
{
    particles_instance_.add_particle(
        transformation_matrix,
        billboard_sequence_,
        velocity,
        air_resistance,
        texture_layer);
}
