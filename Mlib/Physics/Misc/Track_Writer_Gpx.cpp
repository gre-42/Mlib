#include "Track_Writer_Gpx.hpp"
#include <Mlib/Physics/Misc/Track_Element.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

TrackWriterGpx::TrackWriterGpx(const std::string& filename)
: filename_{filename},
  ofstr_{create_ofstream(filename)}
{
    if (ofstr_->fail()) {
        THROW_OR_ABORT("Could not open gpx file for write \"" + filename + '"');
    }
    ofstr_->precision(10);
    *ofstr_ << R"(<?xml version="1.0" encoding="UTF-8" standalone="no" ?>
<gpx version="1.1" creator="Vanilla Rally">
  <trk>
    <trkseg>
)";
    if (ofstr_->fail()) {
        THROW_OR_ABORT("Could not write to file " + filename);
    }
}

TrackWriterGpx::~TrackWriterGpx() {
    *ofstr_ << R"(    </trkseg>
  </trk>
</gpx>)";
    ofstr_->flush();
    if (ofstr_->fail()) {
        lerr() << "Could not write to file "  << filename_;
    }
}

void TrackWriterGpx::write(const FixedArray<double, 3>& position) {
    *ofstr_ << "      <trkpt lat=\"" << position(0) << "\" lon=\"" << position(1) << "\">\n";
    *ofstr_ << "        <ele>" << position(2) << "</ele>\n";
    *ofstr_ << "      </trkpt>\n";
    if (ofstr_->fail()) {
        THROW_OR_ABORT("Could not write to file " + filename_);
    }
}

void TrackWriterGpx::flush() {
    ofstr_->flush();
    if (ofstr_->fail()) {
        THROW_OR_ABORT("Could not write to file " + filename_);
    }
}
