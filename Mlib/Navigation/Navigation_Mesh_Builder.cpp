#include "Navigation_Mesh_Builder.hpp"
#include <chrono>
#include <iostream>
#include <stdexcept>

using namespace Mlib;

NavigationMeshBuilder::NavigationMeshBuilder(
    const Utf8Path& filename,
    const NavigationMeshConfig& cfg)
    : ssm{ctx_, geom_}
{
    if (!geom_.load(&ctx_, filename)) {
        throw std::runtime_error("Could not load obj file");
    }
    ssm.m_cellSize = cfg.cell_size;
    ssm.m_agentRadius = cfg.agent_radius;
    if (!ssm.build()) {
        throw std::runtime_error("Build failed");
    }
}

NavigationMeshBuilder::NavigationMeshBuilder(
    const IndexedFaceSet<float, float, size_t>& indexed_face_set,
    const NavigationMeshConfig& cfg)
: ssm{ctx_, geom_}
{
    if (!geom_.load(&ctx_, indexed_face_set)) {
        throw std::runtime_error("Could not import indexed face set");
    }
    ssm.m_cellSize = cfg.cell_size;
    ssm.m_agentRadius = cfg.agent_radius;
    if (!ssm.build()) {
        throw std::runtime_error("Build failed");
    }
}

NavigationMeshBuilder::~NavigationMeshBuilder() = default;
