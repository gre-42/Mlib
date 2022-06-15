#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Physics/Collision/Typed_Mesh.hpp>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <vector>

namespace Mlib {

class RigidBodyVehicle;
template <class TData, size_t n>
class PlaneNd;
class TransformedMesh;

class SatTracker {
public:
    void get_collision_plane(
        const RigidBodyVehicle& o0,
        const RigidBodyVehicle& o1,
        const std::shared_ptr<TransformedMesh>& mesh0,
        const std::shared_ptr<TransformedMesh>& mesh1,
        double& min_overlap,
        PlaneNd<double, 3>& plane) const;
private:
    mutable std::map<
        const RigidBodyVehicle*,
        std::map<
            const RigidBodyVehicle*,
            std::map<
                std::shared_ptr<TransformedMesh>,
                std::map<std::shared_ptr<TransformedMesh>,
                    std::pair<double, PlaneNd<double, 3>>>>>> collision_planes_;
};

}
