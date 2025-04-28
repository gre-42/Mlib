#include "Subdivided_Way_Vertex.hpp"
#include <Mlib/Math/Blend.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>

using namespace Mlib;

FixedArray<CompressedScenePos, 2> SubdividedWayVertex::position() const {
    return blend(n0.position, n1.position, a0, a1);
}
