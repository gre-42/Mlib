#include "Track_Reader.hpp"
#include <Mlib/Assert.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

TrackReader::TrackReader(
    const std::string& filename,
    size_t nlaps,
    const TransformationMatrix<double, double, 3>* inverse_geographic_mapping,
    TrackElementInterpolationKey interpolation_key)
: ifstr_{create_ifstream(filename)},
  filename_{filename},
  frame_id_{0},
  lap_id_{0},
  nlaps_remaining_{nlaps},
  inverse_geographic_mapping_{inverse_geographic_mapping},
  interpolation_key_{interpolation_key},
  progress_{0.},
  track_element0_{std::nullopt},
  track_element1_{std::nullopt}
{
    if (nlaps == 0) {
        THROW_OR_ABORT("Number of laps must be at least 1");
    }
    if (ifstr_->fail()) {
        THROW_OR_ABORT("Could not open track reader file \"" + filename + '"');
    }
}

TrackReader::~TrackReader() = default;

bool TrackReader::read(float dprogress) {
    if (inverse_geographic_mapping_ == nullptr) {
        THROW_OR_ABORT("TrackReader::read without geographic mapping");
    }
    if (!ifstr_->eof()) {
        while(!track_element1_.has_value() || (track_element1_.value().progress(interpolation_key_) < progress_))
        {
            if (track_element1_.has_value()) {
                track_element0_ = track_element1_;
            }
            track_element1_ = TrackElementExtended::from_stream(track_element1_, *ifstr_, *inverse_geographic_mapping_);
            if (ifstr_->fail()) {
                if (!ifstr_->eof()) {
                    THROW_OR_ABORT("Could not read from file \"" + filename_ + '"');
                }
                if (nlaps_remaining_ == 0) {
                    return false;
                } else {
                    ++lap_id_;
                    if (nlaps_remaining_ != SIZE_MAX) {
                        --nlaps_remaining_;
                    }
                    if (nlaps_remaining_ == 0) {
                        return false;
                    }
                    if (!track_element1_.has_value()) {
                        THROW_OR_ABORT("Received empty and periodic track");
                    }
                    // This assumes that the last element of the track equals the first element,
                    // i.e. that the start the track looks like [0 1 2 3 4 0].
                    // Note that "nlaps_remaining_" is not reset.
                    ifstr_->clear();
                    ifstr_->seekg(0);
                    progress_ = 0.;
                    track_element0_ = std::nullopt;
                    track_element1_ = std::nullopt;
                    frame_id_ = 0;
                    continue;
                }
            } else {
                ++frame_id_;
            }
            if (!track_element0_.has_value()) {
                track_element0_ = track_element1_;
            }
        }
        if (track_element1_.value().progress(interpolation_key_) == track_element0_.value().progress(interpolation_key_)) {
            track_element_ = track_element0_.value();
        } else {
            float alpha = float(
                (progress_ - track_element0_.value().progress(interpolation_key_)) /
                (track_element1_.value().progress(interpolation_key_) - track_element0_.value().progress(interpolation_key_)));
            assert_true(alpha >= 0);
            assert_true(alpha <= 1);
            track_element_ = interpolated(track_element0_.value(), track_element1_.value(), alpha);
        }
        progress_ += dprogress;
        return true;
    }
    return false;
}

bool TrackReader::eof() const {
    return (nlaps_remaining_ == 0) && ifstr_->eof();
}
