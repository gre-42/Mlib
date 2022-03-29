#pragma once
#include <Mlib/Navigation/InputGeom.hpp>
#include <Mlib/Navigation/Sample_SoloMesh.hpp>
#include <Mlib/Navigation/StderrContext.hpp>
#include <string>

namespace Mlib {

template <class TData, class TIndex>
class IndexedFaceSet;

class NavigationMeshBuilder {
public:
    explicit NavigationMeshBuilder(const std::string& filename);
    explicit NavigationMeshBuilder(const IndexedFaceSet<float, size_t>& indexed_face_set);
private:
    StderrContext ctx_;
    InputGeom geom_;
public:
    Sample_SoloMesh ssm;
};

}
