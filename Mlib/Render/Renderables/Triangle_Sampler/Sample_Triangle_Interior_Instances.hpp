#pragma once
#include <Mlib/Geometry/Intersection/Bvh_Fwd.hpp>
#include <Mlib/Geometry/Mesh/Triangle_Sampler2.hpp>
#include <Mlib/Render/Renderables/Triangle_Sampler/Resource_Name_Cycle.hpp>
#include <Mlib/Render/Renderables/Triangle_Sampler/Terrain_Style.hpp>
#include <Mlib/Scene_Precision.hpp>
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
enum class UpAxis;

class TriangleInteriorInstancesSampler {
public:
    TriangleInteriorInstancesSampler(
        const TerrainStyle& terrain_style,
        ScenePos scale,
        UpAxis up_axis,
        const Bvh<CompressedScenePos, 3, FixedArray<CompressedScenePos, 3, 3>>* boundary_bvh,
        const Array<float>& dirtmap,
        float dirtmap_scale,
        const Array<float>& mudmap);
    void sample_triangle(
        const FixedArray<ColoredVertex<CompressedScenePos>, 3>& t,
        unsigned int seed,
        const std::function<void(
            const FixedArray<CompressedScenePos, 3>& p,
            const ParsedResourceName& prn)>& f);
private:
    const TerrainStyleConfig& tsc_;
    TerrainStyleDistancesToBdry distances_to_bdry_;
    ResourceNameCycle rnc_valley_regular_;
    ResourceNameCycle rnc_mountain_regular_;
    ResourceNameCycle rnc_valley_dirt_;
    ResourceNameCycle rnc_mountain_dirt_;
    CompressedScenePos max_dboundary_;
    ScenePos min_dboundary2_;
    TriangleSampler2<ScenePos> ts_;
    ScenePos scale_;
    UpAxis up_axis_;
    const Bvh<CompressedScenePos, 3, FixedArray<CompressedScenePos, 3, 3>>* boundary_bvh_;
    const Array<float>& dirtmap_;
    float dirtmap_scale_;
    const Array<float>& mudmap_;
};

}
