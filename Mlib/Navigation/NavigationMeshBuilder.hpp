#pragma once
#include <Mlib/Navigation/InputGeom.hpp>
#include <Mlib/Navigation/Sample_SoloMesh.hpp>
#include <Mlib/Navigation/StderrContext.hpp>
#include <string>

namespace Mlib {

template <class TDir, class TPos, class TIndex>
class IndexedFaceSet;

struct NavigationMeshConfig {
    float cell_size;
    float agent_radius;
};

class NavigationMeshBuilder {
public:
    explicit NavigationMeshBuilder(
        const std::string& filename,
        const NavigationMeshConfig& cfg);
    explicit NavigationMeshBuilder(
        const IndexedFaceSet<float, float, size_t>& indexed_face_set,
        const NavigationMeshConfig& cfg);
    ~NavigationMeshBuilder();
private:
    StderrContext ctx_;
    InputGeom geom_;
public:
    Sample_SoloMesh ssm;
};

}
