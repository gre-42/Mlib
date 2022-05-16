#include "Track_Reader.hpp"
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>

using namespace Mlib;

TrackReader::TrackReader(
    const std::string& filename,
    const TransformationMatrix<double, 3>* inverse_geographic_mapping,
    float delta_index)
: ifstr_{filename},
  filename_{filename},
  inverse_geographic_mapping_{inverse_geographic_mapping},
  delta_index_{delta_index},
  findex_{0},
  iindex_{0},
  track_element0_{TrackElement::nan()},
  track_element1_{TrackElement::nan()}
{
    if (ifstr_.fail()) {
        throw std::runtime_error("Could not open file \"" + filename + '"');
    }
}

bool TrackReader::read(TrackElement& track_element) {
    if (inverse_geographic_mapping_ == nullptr) {
        throw std::runtime_error("TrackReader::read without geographic mapping");
    }
    if (!ifstr_.eof()) {
        while(iindex_ < std::floor(findex_) + 1) {
            if (iindex_ != 0) {
                track_element0_ = track_element1_;
            }
            track_element1_ = TrackElement::from_stream(ifstr_, *inverse_geographic_mapping_);
            if (ifstr_.fail()) {
                if (!ifstr_.eof()) {
                    throw std::runtime_error("Could not read from file \"" + filename_ + '"');
                }
                return false;
            }
            if (iindex_ == 0) {
                track_element0_ = track_element1_;
            }
            ++iindex_;
        }
        float alpha = 1 - (iindex_ - findex_);
        assert_true(alpha >= 0);
        assert_true(alpha <= 1);
        track_element = interpolated(track_element0_, track_element1_, alpha);
        findex_ += delta_index_;
        return true;
    }
    return false;
}

bool TrackReader::eof() const {
    return ifstr_.eof();
}

void TrackReader::restart() {
    ifstr_.clear();
    ifstr_.seekg(0);
    findex_ = 0;
    iindex_ = 0;
    track_element0_ = TrackElement::nan();
    track_element1_ = TrackElement::nan();
}
