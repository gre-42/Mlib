#include "Navigate.hpp"
#include <Mlib/Geometry/Mesh/Points_And_Adjacency_Impl.hpp>
#include <Mlib/Scene_Graph/Interfaces/Way_Points.hpp>

using namespace Mlib;

Navigate::Navigate() = default;

Navigate::~Navigate() = default;

bool Navigate::has_way_points() const {
    return (way_points_ != nullptr) && !way_points_->empty();
}

const WayPointSandboxesAndBvh& Navigate::way_points() const {
    if (way_points_ == nullptr) {
        THROW_OR_ABORT("Waypoints not set");
    }
    return *way_points_;
}

void Navigate::set_way_points(
    const TransformationMatrix<SceneDir, ScenePos, 3>& absolute_model_matrix,
    const WayPointSandboxes& way_points)
{
    auto w = std::make_shared<WayPointSandboxesAndBvh>();
    for (const auto& [l, wps] : way_points) {
        PointsAndAdjacencyResource t = wps;
        t.transform(absolute_model_matrix.casted<SceneDir, CompressedScenePos>());
        w->add(l, std::make_shared<WayPointsAndBvh>(t));
    }
    way_points_ = std::move(w);
}
