#pragma once
#include <cstddef>
#include <set>

namespace Mlib {

template <class TData, size_t... tshape>
class FixedArray;
template <class TData, size_t... tshape>
class OrderableFixedArray;
class IIntersectableMesh;
struct CollisionTriangleSphere;

class CollisionVertices {
public:
    using Vertices = std::set<OrderableFixedArray<double, 3>>;
    using const_iterator = Vertices::const_iterator;
    void insert(const FixedArray<FixedArray<double, 3>, 3>& tri);
    void insert(const FixedArray<FixedArray<double, 3>, 2>& line);
    void insert(const FixedArray<double, 3>& vertex);
    const_iterator begin() const;
    const_iterator end() const;
private:
    Vertices vertices_;
};

// double get_overlap(
//     const CollisionTriangleSphere& t0,
//     const IIntersectableMesh& mesh1);

double sat_overlap_signed(
    const FixedArray<double, 3>& n,
    const CollisionVertices& vertices0,
    const CollisionVertices& vertices1);

void sat_overlap_unsigned(
    const FixedArray<double, 3>& l,
    const CollisionVertices& vertices0,
    const CollisionVertices& vertices1,
    double& overlap0,
    double& overlap1);

}
