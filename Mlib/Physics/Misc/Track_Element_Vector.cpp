#include "Track_Element_Vector.hpp"
#include <Mlib/Physics/Misc/Track_Element_Extended.hpp>

using namespace Mlib;

TrackElementVector::TrackElementVector(std::vector<std::vector<double>>&& track)
: track_{std::move(track)},
  i_{0}
{}

TrackElementVector::~TrackElementVector() = default;

TrackElementExtended TrackElementVector::read(
    const std::optional<TrackElementExtended>& predecessor,
    const TransformationMatrix<double, double, 3>& inverse_geographic_mapping,
    size_t ntransformations)
{
    if (i_ == track_.size()) {
        ++i_;
        return TrackElementExtended{};
    }
    if (i_ > track_.size()) {
        THROW_OR_ABORT("Attempt to read past the end of the track");
    }
    return TrackElementExtended::create(
        predecessor,
        TrackElement::from_vector(track_[i_++], inverse_geographic_mapping, ntransformations));
}

bool TrackElementVector::eof() const {
    return i_ > track_.size();
}

void TrackElementVector::restart() {
    i_ = 0;
}
