#include "Track_Reader.hpp"
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Physics/Units.hpp>

using namespace Mlib;

TrackReader::TrackReader(
    const std::string& filename,
    bool periodic,
    const TransformationMatrix<double, double, 3>* inverse_geographic_mapping)
: ifstr_{filename},
  filename_{filename},
  periodic_{periodic},
  inverse_geographic_mapping_{inverse_geographic_mapping},
  elapsed_seconds_{0.f},
  track_element0_{TrackElement::nan()},
  track_element1_{TrackElement::nan()}
{
    if (ifstr_.fail()) {
        throw std::runtime_error("Could not open file \"" + filename + '"');
    }
}

bool TrackReader::read(TrackElement& track_element, size_t& nperiods, float dt) {
    if (inverse_geographic_mapping_ == nullptr) {
        throw std::runtime_error("TrackReader::read without geographic mapping");
    }
    if (!ifstr_.eof()) {
        while(std::isnan(track_element1_.elapsed_seconds) || (track_element1_.elapsed_seconds < elapsed_seconds_))
        {
            if (!std::isnan(track_element1_.elapsed_seconds)) {
                track_element0_ = track_element1_;
            }
            track_element1_ = TrackElement::from_stream(ifstr_, *inverse_geographic_mapping_);
            if (ifstr_.fail()) {
                if (!ifstr_.eof()) {
                    throw std::runtime_error("Could not read from file \"" + filename_ + '"');
                }
                if (periodic_) {
                    if (std::isnan(track_element1_.elapsed_seconds)) {
                        throw std::runtime_error("Received empty and periodic track");
                    }
                    ++nperiods;
                    restart();
                    continue;
                } else {
                    return false;
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
    return !periodic_ && ifstr_.eof();
}

void TrackReader::restart() {
    ifstr_.clear();
    ifstr_.seekg(0);
    elapsed_seconds_ = 0.f;
    track_element0_ = TrackElement::nan();
    track_element1_ = TrackElement::nan();
}
