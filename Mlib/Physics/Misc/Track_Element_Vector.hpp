#pragma once
#include <Mlib/Physics/Misc/ITrack_Element_Sequence.hpp>
#include <cstddef>
#include <vector>

namespace Mlib {

class TrackElementVector: public ITrackElementSequence {
public:
    explicit TrackElementVector(std::vector<std::vector<double>>&& track);
    ~TrackElementVector();
    virtual TrackElementExtended read(
        const std::optional<TrackElementExtended>& predecessor,
        const TransformationMatrix<double, double, 3>& inverse_geographic_mapping,
        size_t ntransformations) override;
    virtual bool eof() const override;
    virtual void restart() override;
private:
    std::vector<std::vector<double>> track_;
    size_t i_;
};

}
