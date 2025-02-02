#include "Track_Writer.hpp"
#include <Mlib/Os/Os.hpp>
#include <Mlib/Physics/Misc/Track_Element.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

TrackWriter::TrackWriter(
    const std::string& filename,
    const TransformationMatrix<double, double, 3>* geographic_mapping)
    : filename_{ filename }
    , geographic_mapping_{ geographic_mapping }
    , ofstr_{ create_ofstream(filename) }
{
    if (ofstr_->fail()) {
        THROW_OR_ABORT("Could not open track file for write \"" + filename + '"');
    }
}

TrackWriter::~TrackWriter() = default;

void TrackWriter::write(const TrackElement& e)
{
    if (geographic_mapping_ == nullptr) {
        THROW_OR_ABORT("TrackWriter::write without geographic mapping");
    }
    e.write_to_stream(*ofstr_, *geographic_mapping_);
    *ofstr_ << '\n';
}

void TrackWriter::flush() {
    ofstr_->flush();
    if (ofstr_->fail()) {
        THROW_OR_ABORT("Could not write to file " + filename_);
    }
}
