#pragma once
#include <cstddef>
#include <optional>

namespace Mlib {

template <class TDir, class TPos, size_t n>
class TransformationMatrix;
struct TrackElementExtended;

class ITrackElementSequence {
public:
    virtual ~ITrackElementSequence() = default;
    virtual TrackElementExtended read(
        const std::optional<TrackElementExtended>& predecessor,
        const TransformationMatrix<double, double, 3>& inverse_geographic_mapping,
        size_t ntransformations) = 0;
    virtual bool eof() const = 0;
    virtual void restart() = 0;
};

}
