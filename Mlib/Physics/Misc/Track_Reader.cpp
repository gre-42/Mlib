#include "Track_Reader.hpp"
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Os.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

TrackReader::TrackReader(
    const std::string& filename,
    size_t nlaps,
    const TransformationMatrix<double, double, 3>* inverse_geographic_mapping)
: ifstr_{create_ifstream(filename)},
  filename_{filename},
  nlaps_remaining_{nlaps},
  inverse_geographic_mapping_{inverse_geographic_mapping},
  elapsed_seconds_{0.f},
  track_element0_{TrackElement::nan()},
  track_element1_{TrackElement::nan()}
{
    if (nlaps == 0) {
        THROW_OR_ABORT("Number of laps must be at least 1");
    }
    if (ifstr_->fail()) {
        THROW_OR_ABORT("Could not open track reader file \"" + filename + '"');
    }
}

TrackReader::~TrackReader() = default;

bool TrackReader::read(TrackElement& track_element, size_t& nlaps, float dt) {
    if (inverse_geographic_mapping_ == nullptr) {
        THROW_OR_ABORT("TrackReader::read without geographic mapping");
    }
    if (!ifstr_->eof()) {
        while(std::isnan(track_element1_.elapsed_seconds) || (track_element1_.elapsed_seconds < elapsed_seconds_))
        {
            if (!std::isnan(track_element1_.elapsed_seconds)) {
                track_element0_ = track_element1_;
            }
            track_element1_ = TrackElement::from_stream(*ifstr_, *inverse_geographic_mapping_);
            if (ifstr_->fail()) {
                if (!ifstr_->eof()) {
                    THROW_OR_ABORT("Could not read from file \"" + filename_ + '"');
                }
                if (nlaps_remaining_ == 0) {
                    return false;
                } else {
                    ++nlaps;
                    if (nlaps_remaining_ != SIZE_MAX) {
                        --nlaps_remaining_;
                    }
                    if (nlaps_remaining_ == 0) {
                        return false;
                    }
                    if (std::isnan(track_element1_.elapsed_seconds)) {
                        THROW_OR_ABORT("Received empty and periodic track");
                    }
                    restart();
                    continue;
                }
            }
            if (std::isnan(track_element0_.elapsed_seconds)) {
                track_element0_ = track_element1_;
            }
        }
        if (track_element1_.elapsed_seconds == track_element0_.elapsed_seconds) {
            track_element = track_element0_;
        } else {
            float alpha = (elapsed_seconds_ - track_element0_.elapsed_seconds) / (track_element1_.elapsed_seconds - track_element0_.elapsed_seconds);
            assert_true(alpha >= 0);
            assert_true(alpha <= 1);
            track_element = interpolated(track_element0_, track_element1_, alpha);
        }
        elapsed_seconds_ += dt / s;
        return true;
    }
    return false;
}

bool TrackReader::eof() const {
    return (nlaps_remaining_ == 0) && ifstr_->eof();
}

void TrackReader::restart() {
    ifstr_->clear();
    ifstr_->seekg(0);
    elapsed_seconds_ = 0.f;
    track_element0_ = TrackElement::nan();
    track_element1_ = TrackElement::nan();
}
