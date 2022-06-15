#include "Track_Writer.hpp"
#include <Mlib/Physics/Misc/Track_Element.hpp>

using namespace Mlib;

TrackWriter::TrackWriter(
    const std::string& filename,
    const TransformationMatrix<double, double, 3>* geographic_mapping)
: filename_{filename},
  geographic_mapping_{geographic_mapping}
{
    ofstr_.open(filename);
    if (ofstr_.fail()) {
        throw std::runtime_error("Could not open file " + filename);
    }
}

void TrackWriter::write(const TrackElement& e)
{
    if (geographic_mapping_ == nullptr) {
        throw std::runtime_error("TrackWriter::write without geographic mapping");
    }
    e.write_to_stream(ofstr_, *geographic_mapping_);
    ofstr_ << std::endl;
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
