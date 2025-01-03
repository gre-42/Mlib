#pragma once
#include <Mlib/Scene_Precision.hpp>
#include <functional>
#include <list>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;

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
	const std::list<FixedArray<CompressedScenePos, 2>>& path,
	const std::function<void(
        const FixedArray<CompressedScenePos, 2>& aL,
        const FixedArray<CompressedScenePos, 2>& aR,
        const FixedArray<CompressedScenePos, 2>& b,
        const FixedArray<CompressedScenePos, 2>& c,
        const FixedArray<CompressedScenePos, 2>& dL,
        const FixedArray<CompressedScenePos, 2>& dR,
        SegmentPosition position)>& visit);

}
