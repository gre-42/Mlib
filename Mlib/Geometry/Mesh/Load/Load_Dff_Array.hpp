#pragma once
#include <iosfwd>
#include <list>
#include <memory>
#include <string>

namespace Mlib {

template <class TPos>
class ColoredVertexArray;
template <class TPos>
struct LoadMeshConfig;
class DrawDistanceDb;

enum class FrameTransformation {
    KEEP = 0,
    ZERO_POSITION = 1 << 0,
    IDENTITY_ROTATION = 1 << 1
};

inline bool any(FrameTransformation t) {
    return t != FrameTransformation::KEEP;
}

inline FrameTransformation operator & (FrameTransformation a, FrameTransformation b) {
    return (FrameTransformation)((int)a & (int)b);
}

inline FrameTransformation operator | (FrameTransformation a, FrameTransformation b) {
    return (FrameTransformation)((int)a | (int)b);
}

template <class TPosition>
struct DffArrays {
    std::list<std::shared_ptr<ColoredVertexArray<TPosition>>> renderables;
};

template <class TPosition>
DffArrays<TPosition> load_dff(
    const std::string& filename,
    const LoadMeshConfig<TPosition>& cfg,
    const DrawDistanceDb& dddb,
    FrameTransformation frame_transformation);

template <class TPosition>
DffArrays<TPosition> load_dff(
    std::istream& istr,
    const std::string& name,
    const LoadMeshConfig<TPosition>& cfg,
    const DrawDistanceDb& dddb,
    FrameTransformation frame_transformation);

}
