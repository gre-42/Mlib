#pragma once
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <Mlib/Scene_Pos.hpp>
#include <cstddef>
#include <set>

namespace Mlib {

template <class TData, size_t... tshape>
class FixedArray;

class CollisionVertices {
public:
    using Vertices = std::set<OrderableFixedArray<ScenePos, 3>>;
    using const_iterator = Vertices::const_iterator;
    void insert(const FixedArray<ScenePos, 4, 3>& quad);
    void insert(const FixedArray<ScenePos, 3, 3>& tri);
    void insert(const FixedArray<ScenePos, 2, 3>& line);
    void insert(const FixedArray<ScenePos, 3>& vertex);
    const_iterator begin() const;
    const_iterator end() const;
    inline const Vertices& get() const {
        return vertices_;
    }
private:
    Vertices vertices_;
};

}
