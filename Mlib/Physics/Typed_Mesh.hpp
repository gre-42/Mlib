#pragma once
#include <utility>

namespace Mlib {

enum class MeshType {
    CHASSIS,
    TIRE_LINE
};

template <class T>
struct TypedMesh {
    MeshType mesh_type;
    T mesh;
    bool operator < (const TypedMesh& other) const {
        return std::make_pair(mesh_type, mesh) < std::make_pair(other.mesh_type, other.mesh);
    }
};

}
