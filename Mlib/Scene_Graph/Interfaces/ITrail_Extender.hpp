#pragma once
#include <Mlib/Scene_Precision.hpp>
#include <cstddef>

namespace Mlib {

template <class TDir, class TPos, size_t n>
class TransformationMatrix;

enum class TrailLocationType {
    MIDPOINT,
    ENDPOINT
};

class ITrailExtender {
public:
    virtual ~ITrailExtender() = default;
    virtual void append_location(
        const TransformationMatrix<float, ScenePos, 3>& location,
        TrailLocationType location_type) = 0;

};

}
