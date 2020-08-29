#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <fstream>

namespace Mlib {

class TrackReader {
public:
    explicit TrackReader(const std::string& filename);
    bool read(float& time, FixedArray<float, 3>& position, FixedArray<float, 3>& rotation);
    bool eof() const;
    void restart();
private:
    std::ifstream ifstr_;
    std::string filename_;
};

}
