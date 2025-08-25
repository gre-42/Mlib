#pragma once
#include <Mlib/Scene_Graph/Interfaces/Way_Points_Fwd.hpp>
#include <memory>

namespace Mlib {

class Navigate {
public:
    Navigate();
    ~Navigate();
    bool has_way_points() const;
    const WayPointSandboxesAndBvh& way_points() const;
    const WayPointsAndBvh& way_points(JoinedWayPointSandbox key) const;
    void set_way_points(
        const TransformationMatrix<SceneDir, ScenePos, 3>& absolute_model_matrix,
        const WayPointSandboxes& way_points);
private:
    std::shared_ptr<WayPointSandboxesAndBvh> way_points_;
};

}
