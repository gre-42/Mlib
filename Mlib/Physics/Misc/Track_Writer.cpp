#include "Track_Writer.hpp"
#include <Mlib/Physics/Misc/Track_Element.hpp>

using namespace Mlib;

TrackWriter::TrackWriter(const std::string& filename)
: filename_{filename}
{
    ofstr_.open(filename);
    if (ofstr_.fail()) {
        throw std::runtime_error("Could not open file " + filename);
    }
}

void TrackWriter::write(const TrackElement& e) {
    ofstr_ << e << std::endl;
    if (ofstr_.fail()) {
        throw std::runtime_error("Could not write to file " + filename_);
    }
}

void TrackWriter::flush() {
    ofstr_.flush();
    if (ofstr_.fail()) {
        throw std::runtime_error("Could not write to file " + filename_);
    }
}
