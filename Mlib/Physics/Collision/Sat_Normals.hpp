#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Physics/Collision/Typed_Mesh.hpp>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <vector>

namespace Mlib {

class RigidBody;
template <class TData, size_t n>
class PlaneNd;
class TransformedMesh;

class SatTracker {
public:
    void get_collision_plane(
        const RigidBody& o0,
        const RigidBody& o1,
        const std::shared_ptr<TransformedMesh>& mesh0,
        const std::shared_ptr<TransformedMesh>& mesh1,
        float& min_overlap,
        PlaneNd<float, 3>& plane) const;
private:
    mutable std::map<
        const RigidBody*,
        std::map<
            const RigidBody*,
            std::map<
                std::shared_ptr<TransformedMesh>,
                std::map<std::shared_ptr<TransformedMesh>,
                    std::pair<float, PlaneNd<float, 3>>>>>> collision_planes_;
};

}
