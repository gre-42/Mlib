#pragma once
#include <Mlib/Geometry/Mesh/Triangle_Sampler2.hpp>
#include <Mlib/Render/Renderables/Triangle_Sampler/Resource_Name_Cycle.hpp>
#include <Mlib/Render/Renderables/Triangle_Sampler/Terrain_Style.hpp>
#include <list>
#include <memory>

namespace Mlib {

class TerrainStyle;
struct TerrainStyleConfig;
template <class TData>
class Array;
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
enum class UpAxis;

class TriangleInteriorInstancesSampler {
public:
    TriangleInteriorInstancesSampler(
        const TerrainStyle& terrain_style,
        double scale,
        UpAxis up_axis,
        const Bvh<double, FixedArray<FixedArray<double, 3>, 3>, 3>* boundary_bvh,
        const Array<float>& dirtmap,
        float dirtmap_scale,
        const Array<float>& mudmap);
    void sample_triangle(
        const FixedArray<ColoredVertex<double>, 3>& t,
        unsigned int seed,
        const std::function<void(
            const FixedArray<double, 3>& p,
            const ParsedResourceName& prn)>& f);
private:
    const TerrainStyleConfig& tsc_;
    TerrainStyleDistancesToBdry distances_to_bdry_;
    ResourceNameCycle rnc_valley_regular_;
    ResourceNameCycle rnc_mountain_regular_;
    ResourceNameCycle rnc_valley_dirt_;
    ResourceNameCycle rnc_mountain_dirt_;
    double max_dboundary_;
    double min_dboundary2_;
    TriangleSampler2<double> ts_;
    double scale_;
    UpAxis up_axis_;
    const Bvh<double, FixedArray<FixedArray<double, 3>, 3>, 3>* boundary_bvh_;
    const Array<float>& dirtmap_;
    float dirtmap_scale_;
    const Array<float>& mudmap_;
};

}
