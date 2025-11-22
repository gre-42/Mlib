#pragma once
#include <Mlib/Scene_Config/Scene_Precision.hpp>
#include <functional>
#include <list>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
struct SubdividedWayVertex;

enum class SegmentPosition {
    MIDDLE = 0,
    START = 1 << 0,
    END = 1 << 1,
};

inline SegmentPosition operator | (SegmentPosition a, SegmentPosition b) {
    return (SegmentPosition)((int)a | (int)b);
}

inline SegmentPosition operator & (SegmentPosition a, SegmentPosition b) {
    return (SegmentPosition)((int)a & (int)b);
}

inline bool any(SegmentPosition a) {
    return a != SegmentPosition::MIDDLE;
}

void visit_line_segments(
	const std::list<SubdividedWayVertex>& path,
	const std::function<void(
        const SubdividedWayVertex& aL,
        const SubdividedWayVertex& aR,
        const SubdividedWayVertex& b,
        const SubdividedWayVertex& c,
        const SubdividedWayVertex& dL,
        const SubdividedWayVertex& dR,
        SegmentPosition position)>& visit);

}
