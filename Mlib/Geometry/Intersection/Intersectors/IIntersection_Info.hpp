#pragma once
#include <Mlib/Scene_Pos.hpp>
#include <cstddef>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;

class IIntersectionInfo {
public:
    virtual ~IIntersectionInfo() = default;
    virtual bool has_normal_and_overlap() const = 0;
    virtual ScenePos ray_t() const = 0;
    virtual FixedArray<ScenePos, 3> intersection_point() const = 0;
    virtual FixedArray<ScenePos, 3> normal0() const = 0;
    virtual FixedArray<ScenePos, 3> normal() const = 0;
    virtual ScenePos overlap() const = 0;
};

}
