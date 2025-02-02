#include "NavigationMeshBuilder.hpp"
#include <Mlib/Throw_Or_Abort.hpp>
#include <chrono>
#include <iostream>

using namespace Mlib;

NavigationMeshBuilder::NavigationMeshBuilder(
    const std::string& filename,
    const NavigationMeshConfig& cfg)
: ssm{ctx_, geom_}
{
    if (!geom_.load(&ctx_, filename)) {
        THROW_OR_ABORT("Could not load obj file");
    }
    ssm.m_cellSize = cfg.cell_size;
    ssm.m_agentRadius = cfg.agent_radius;
    if (!ssm.build()) {
        THROW_OR_ABORT("Build failed");
    }
}

NavigationMeshBuilder::NavigationMeshBuilder(
    const IndexedFaceSet<float, float, size_t>& indexed_face_set,
    const NavigationMeshConfig& cfg)
: ssm{ctx_, geom_}
{
    if (!geom_.load(&ctx_, indexed_face_set)) {
        THROW_OR_ABORT("Could not import indexed face set");
    }
    ssm.m_cellSize = cfg.cell_size;
    ssm.m_agentRadius = cfg.agent_radius;
    if (!ssm.build()) {
        THROW_OR_ABORT("Build failed");
    }
}

NavigationMeshBuilder::~NavigationMeshBuilder() = default;
