#include "Track_Reader.hpp"
#include <Mlib/Array/Fixed_Array.hpp>

using namespace Mlib;

TrackReader::TrackReader(const std::string& filename)
: ifstr_{filename},
  filename_{filename}
{
    if (ifstr_.fail()) {
        throw std::runtime_error("Could not open file \"" + filename + '"');
    }
}

bool TrackReader::read(float& time, FixedArray<float, 3>& position, FixedArray<float, 3>& rotation) {
    if (!ifstr_.eof()) {
        ifstr_ >> time >> position(0) >> position(1) >> position(2) >> rotation(0) >> rotation(1) >> rotation(2);
        if (ifstr_.fail()) {
            if (!ifstr_.eof()) {
                throw std::runtime_error("Could not read from file \"" + filename_ + '"');
            }
            return false;
        }
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
}
