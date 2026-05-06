#include "Scene_Graph.hpp"
#include <Mlib/Geometry/Mesh/IIntersectable_Mesh.hpp>
#include <Mlib/Scene_Config/Physics_Engine_Config.hpp>
#include <Mlib/Scene_Config/Render_Engine_Config.hpp>
#include <stdexcept>

using namespace Mlib;

SceneGraph::SceneGraph(
    const RenderEngineConfig& rcfg,
    const PhysicsEngineConfig& pcfg)
    : dynamic_renderables_{
        {rcfg.bvh_max_size, rcfg.bvh_max_size, rcfg.bvh_max_size},
        rcfg.bvh_levels}
    , static_renderables_{
        {rcfg.bvh_max_size, rcfg.bvh_max_size, rcfg.bvh_max_size},
        rcfg.bvh_levels}
    , dynamic_object_collidables_{
        {pcfg.bvh_max_size, pcfg.bvh_max_size, pcfg.bvh_max_size},
        pcfg.bvh_levels,
        pcfg.grid_level,
        pcfg.ncells,
        {pcfg.dilation_radius, pcfg.dilation_radius, pcfg.dilation_radius}}
    , static_polygonal_collidables_{
        {pcfg.bvh_max_size, pcfg.bvh_max_size, pcfg.bvh_max_size},
        pcfg.bvh_levels,
        pcfg.grid_level,
        pcfg.ncells,
        {pcfg.dilation_radius, pcfg.dilation_radius, pcfg.dilation_radius}}
{}

SceneGraph::~SceneGraph() = default;

void SceneGraph::render(const Camera& camera) {

}

void SceneGraph::move(SceneElementTypes types) {
    if (any(types & ~(SceneElementTypes::VISIBLE | SceneElementTypes::COLLIDABLE))) {
        throw std::runtime_error("Unexpected scene element type");
    }
    if (any(types & SceneElementTypes::VISIBLE)) {
        dynamic_renderables_.move_a_lot();
    }
    if (any(types & SceneElementTypes::COLLIDABLE)) {
        dynamic_object_collidables_.move_a_lot();
    }
}

void SceneGraph::render(
    const FixedArray<float, 4, 4>& vp,
    const FixedArray<ScenePos, 3>& offset,
    std::set<DeferredRenderable>& deferred,
    SceneElementTypes types)
{}
