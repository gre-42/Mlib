#pragma once
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <cstddef>
#include <set>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;

class CollisionVertices {
public:
    using Vertices = std::set<OrderableFixedArray<CompressedScenePos, 3>>;
    using const_iterator = Vertices::const_iterator;
    void insert(const FixedArray<CompressedScenePos, 4, 3>& quad);
    void insert(const FixedArray<CompressedScenePos, 3, 3>& tri);
    void insert(const FixedArray<CompressedScenePos, 2, 3>& line);
    void insert(const FixedArray<CompressedScenePos, 3>& vertex);
    const_iterator begin() const;
    const_iterator end() const;
    inline const Vertices& get() const {
        return vertices_;
    }
private:
    Vertices vertices_;
};

}
