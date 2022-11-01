#pragma once
#include <Mlib/Geometry/Mesh/Triangle_Sampler2.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Resource_Name_Cycle.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Terrain_Style.hpp>
#include <list>
#include <memory>

namespace Mlib {

struct TerrainStyle;
struct TerrainStyleConfig;
template <typename TData, size_t... tshape>
class FixedArray;
template <class TPos>
struct ColoredVertex;
template <class TPos>
class ColoredVertexArray;
template <class TDir, class TPos, size_t n>
class TransformationMatrix;
struct SceneGraphConfig;
template <class TData, class TPayload, size_t tndim>
class Bvh;

class TriangleInteriorInstancesSampler {
public:
    TriangleInteriorInstancesSampler(
        const TerrainStyle& terrain_style,
        double scale,
        const Bvh<double, FixedArray<FixedArray<double, 3>, 3>, 3>* boundary_bvh);
    void sample_triangle(
        const FixedArray<ColoredVertex<double>, 3>& t,
        unsigned int seed,
        const std::function<void(
            const FixedArray<double, 3>& p,
            const ParsedResourceName& prn)>& f);
private:
    const TerrainStyleConfig& tsc_;
    TerrainStyleDistancesToBdry distances_to_bdry_;
    ResourceNameCycle rnc_valley_;
    ResourceNameCycle rnc_mountain_;
    double max_dboundary_;
    double min_dboundary2_;
    TriangleSampler2<double> ts_;
    double scale_;
    const Bvh<double, FixedArray<FixedArray<double, 3>, 3>, 3>* boundary_bvh_;
};

}
