#include "NavigationMeshBuilder.hpp"
#include <chrono>
#include <iostream>

using namespace Mlib;

NavigationMeshBuilder::NavigationMeshBuilder(const std::string& filename)
: ssm{ctx_, geom_}
{
    if (!geom_.load(&ctx_, filename)) {
        throw std::runtime_error("Could not load obj file");
    }
    ssm.m_cellSize = 1;
    if (!ssm.build()) {
        throw std::runtime_error("Build failed");
    }
}

NavigationMeshBuilder::NavigationMeshBuilder(const IndexedFaceSet<float, size_t>& indexed_face_set)
: ssm{ctx_, geom_}
{
    if (!geom_.import(&ctx_, indexed_face_set)) {
        throw std::runtime_error("Could not import indexed face set");
    }
    ssm.m_cellSize = 1;
    if (!ssm.build()) {
        throw std::runtime_error("Build failed");
    }
}
