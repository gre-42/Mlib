#include "Trail_Storage.hpp"
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/Trail_Extender.hpp>

using namespace Mlib;

TrailStorage::TrailStorage(
    TrailsInstance& trails_instance,
    TrailSequence trail_sequence,
    const UUVector<FixedArray<ColoredVertex<float>, 3>>& segment,
    ScenePos min_spawn_length,
    ScenePos max_spawn_length,
    float spawn_duration)
    : trails_instance_{ trails_instance }
    , trail_sequence_{ std::move(trail_sequence) }
    , segment_{ segment }
    , min_spawn_length_{ min_spawn_length }
    , max_spawn_length_{ max_spawn_length }
    , spawn_duration_{ spawn_duration }
{}

std::unique_ptr<ITrailExtender> TrailStorage::add_trail_extender() {
    return std::unique_ptr<ITrailExtender>(new TrailExtender(
        trails_instance_,
        trail_sequence_,
        segment_,
        min_spawn_length_,
        max_spawn_length_,
        spawn_duration_));
}
