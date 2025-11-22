#pragma once
#include <Mlib/Scene_Config/Scene_Precision.hpp>
#include <cstddef>
#include <string>

namespace Mlib {

enum class SpawnAction {
    DRY_RUN,
    DO_IT
};

template <class TDir, class TPos, size_t n>
class TransformationMatrix;
template <class TData, size_t tndim>
class AxisAlignedBoundingBox;
class RigidBodyVehicle;

struct GeometrySpawnArguments {
    const TransformationMatrix<SceneDir, CompressedScenePos, 3>& spawn_point;
    const AxisAlignedBoundingBox<CompressedScenePos, 3>& swept_aabb;
    const RigidBodyVehicle* ignored;
    SpawnAction action;
};

struct NodeSpawnArguments {
    std::string suffix;
    bool if_with_graphics;
    bool if_with_physics;
};

}
