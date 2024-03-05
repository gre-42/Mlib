#include "Trail_Storage.hpp"
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/Trail_Extender.hpp>

using namespace Mlib;

TrailStorage::TrailStorage(
    TrailsInstance& trails_instance,
    TrailSequence trail_sequence,
    const std::vector<FixedArray<ColoredVertex<float>, 3>>& segment,
    double minimum_length)
    : trails_instance_{ trails_instance }
    , trail_sequence_{ std::move(trail_sequence) }
    , segment_{ segment }
    , minimum_length_{ minimum_length }
{}

std::unique_ptr<ITrailExtender> TrailStorage::add_trail_extender() {
	return std::unique_ptr<ITrailExtender>(new TrailExtender(
        trails_instance_,
        trail_sequence_,
        segment_,
        minimum_length_));
}
