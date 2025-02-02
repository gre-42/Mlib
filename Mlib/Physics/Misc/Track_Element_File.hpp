#pragma once
#include <Mlib/Physics/Misc/ITrack_Element_Sequence.hpp>
#include <iosfwd>
#include <memory>
#include <string>

namespace Mlib {

class TrackElementFile: public ITrackElementSequence {
public:
    TrackElementFile(
        std::unique_ptr<std::istream>&& istr,
        std::string filename);
    ~TrackElementFile();
    virtual TrackElementExtended read(
        const std::optional<TrackElementExtended>& predecessor,
        const TransformationMatrix<double, double, 3>& inverse_geographic_mapping,
        size_t ntransformations) override;
    virtual bool eof() const override;
    virtual void restart() override;
private:
    std::unique_ptr<std::istream> istr_;
    std::string filename_;
};

}
