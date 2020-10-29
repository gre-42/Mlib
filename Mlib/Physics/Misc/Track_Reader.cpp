#include "Track_Reader.hpp"
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>

using namespace Mlib;

TrackReader::TrackReader(const std::string& filename, float delta_index)
: ifstr_{filename},
  filename_{filename},
  delta_index_{delta_index},
  findex_{0},
  iindex_{0},
  position0_{fixed_nans<float, 3>()},
  rotation0_{fixed_nans<float, 3>()},
  position1_{fixed_nans<float, 3>()},
  rotation1_{fixed_nans<float, 3>()}
{
    if (ifstr_.fail()) {
        throw std::runtime_error("Could not open file \"" + filename + '"');
    }
}

bool TrackReader::read(float& time, FixedArray<float, 3>& position, FixedArray<float, 3>& rotation) {
    if (!ifstr_.eof()) {
        while(iindex_ < std::floor(findex_) + 1) {
            if (iindex_ != 0) {
                position0_ = position1_;
                rotation0_ = rotation1_;
            }
            ifstr_ >> time >> position1_(0) >> position1_(1) >> position1_(2) >> rotation1_(0) >> rotation1_(1) >> rotation1_(2);
            if (ifstr_.fail()) {
                if (!ifstr_.eof()) {
                    throw std::runtime_error("Could not read from file \"" + filename_ + '"');
                }
                return false;
            }
            if (iindex_ == 0) {
                position0_ = position1_;
                rotation0_ = rotation1_;
            }
            ++iindex_;
        }
        float alpha = iindex_ - findex_;
        assert_true(alpha >= 0);
        assert_true(alpha <= 1);
        position = alpha * position0_ + (1 - alpha) * position1_;
        rotation = matrix_2_tait_bryan_angles(
            alpha * tait_bryan_angles_2_matrix(rotation0_) +
            (1 - alpha) * tait_bryan_angles_2_matrix(rotation1_));
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
    position0_ = fixed_nans<float, 3>();
    rotation0_ = fixed_nans<float, 3>();
    position1_ = fixed_nans<float, 3>();
    rotation1_ = fixed_nans<float, 3>();
}
