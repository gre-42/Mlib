#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Physics/Misc/Track_Element.hpp>
#include <fstream>

namespace Mlib {

class TrackReader {
public:
    explicit TrackReader(const std::string& filename, float delta_index = 1);
    bool read(TrackElement& track_element);
    bool eof() const;
    void restart();
private:
    std::ifstream ifstr_;
    std::string filename_;
    float delta_index_;
    float findex_;
    size_t iindex_;
    TrackElement track_element0_;
    TrackElement track_element1_;
};

}
