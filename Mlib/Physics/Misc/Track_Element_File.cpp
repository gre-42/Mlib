#include "Track_Element_File.hpp"
#include <Mlib/Physics/Misc/Track_Element_Extended.hpp>
#include <istream>

using namespace Mlib;

TrackElementFile::TrackElementFile(
    std::unique_ptr<std::istream>&& istr,
    std::string filename)
: istr_{std::move(istr)},
  filename_{std::move(filename)}
{
    if (istr_->fail()) {
        if (filename_.empty()) {
            THROW_OR_ABORT("Could not open track stream");
        } else {
            THROW_OR_ABORT("Could not open track reader file \"" + filename_ + '"');
        }
    }
}

TrackElementFile::~TrackElementFile() = default;

TrackElementExtended TrackElementFile::read(
    const std::optional<TrackElementExtended>& predecessor,
    const TransformationMatrix<double, double, 3>& inverse_geographic_mapping,
    size_t ntransformations)
{
    auto result = TrackElementExtended::create(
        predecessor,
        TrackElement::from_stream(*istr_, inverse_geographic_mapping, ntransformations));
    if (istr_->fail() && !istr_->eof()) {
        if (filename_.empty()) {
            THROW_OR_ABORT("Could not read from track stream");
        } else {
            THROW_OR_ABORT("Could not read from file \"" + filename_ + '"');
        }
    }
    return result;
}

bool TrackElementFile::eof() const {
    return istr_->eof();
}

void TrackElementFile::restart() {
    istr_->clear();
    istr_->seekg(0);
}
