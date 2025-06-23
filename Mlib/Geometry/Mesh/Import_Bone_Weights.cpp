#include "Import_Bone_Weights.hpp"
#include <Mlib/Assert.hpp>
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Geometry/Mesh/Bone_Weight.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>

using namespace Mlib;

struct VertexAndWeights {
    const FixedArray<float, 3>* position;
    const std::vector<BoneWeight>* weights;
};

void Mlib::import_bone_weights(
    AnimatedColoredVertexArrays& dest,
    const AnimatedColoredVertexArrays& source,
    float max_distance)
{
    if (!source.dcvas.empty()) {
        THROW_OR_ABORT("import_bone_weights not implemented for double precision");
    }
    source.check_consistency();
    Bvh<float, 3, VertexAndWeights> bvh{{max_distance / 10, max_distance / 10, max_distance / 10}, 10};
    for (const std::shared_ptr<ColoredVertexArray<float>>& other : source.scvas) {
        auto wo_it = other->triangle_bone_weights.begin();
        for (const auto& t : other->triangles) {
            auto vo_it = wo_it->flat_begin();
            for (const auto& v : t.flat_iterable()) {
                bvh.insert(
                    AxisAlignedBoundingBox<float, 3>::from_point(v.position),
                    VertexAndWeights{ &v.position, vo_it });
                ++vo_it;
            }
            ++wo_it;
        }
    }
    for (const std::shared_ptr<ColoredVertexArray<float>>& cva : dest.scvas) {
        if (!cva->triangle_bone_weights.empty()) {
            THROW_OR_ABORT("import_bone_weights requires empty triangle bone weights");
        }
        cva->triangle_bone_weights.reserve(cva->triangles.size());
        for (const auto& t : cva->triangles) {
            FixedArray<std::vector<BoneWeight>, std::remove_reference_t<decltype(t)>::length()> wn = uninitialized;
            std::vector<BoneWeight>* wn_it = wn.flat_begin();
            for (const auto& v : t.flat_iterable()) {
                float best_distance2 = INFINITY;
                const std::vector<BoneWeight>* best_weights = nullptr;
                bvh.visit(
                    AxisAlignedBoundingBox<float, 3>::from_center_and_radius(v.position, max_distance),
                    [&](const VertexAndWeights& nv)
                {
                    float dist2 = sum(squared(v.position - *nv.position));
                    if (dist2 < best_distance2) {
                        best_distance2 = dist2;
                        best_weights = nv.weights;
                    }
                    return true;
                });
                if (best_weights == nullptr) {
                    THROW_OR_ABORT("Could not find weight");
                }
                *wn_it = *best_weights;
                ++wn_it;
            }
            cva->triangle_bone_weights.emplace_back(wn);
        }
    }
    dest.bone_indices = source.bone_indices;
    dest.skeleton = source.skeleton;
    dest.check_consistency();
}
