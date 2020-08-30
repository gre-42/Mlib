#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <fstream>

namespace Mlib {

class TrackReader {
public:
    explicit TrackReader(const std::string& filename, float delta_index = 1);
    bool read(float& time, FixedArray<float, 3>& position, FixedArray<float, 3>& rotation);
    bool eof() const;
    void restart();
private:
    std::ifstream ifstr_;
    std::string filename_;
    float delta_index_;
    float findex_;
    size_t iindex_;
    FixedArray<float, 3> position0_;
    FixedArray<float, 3> rotation0_;
    FixedArray<float, 3> position1_;
    FixedArray<float, 3> rotation1_;
};

}
