#include "Import_Bone_Weights.hpp"
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>

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
    source.check_consistency();
    Bvh<float, VertexAndWeights, 3> bvh{{max_distance / 10, max_distance / 10, max_distance / 10}, 10};
    for (const std::shared_ptr<ColoredVertexArray>& other : source.cvas) {
        assert_true(other->line_bone_weights.empty());
        auto wo_it = other->triangle_bone_weights.begin();
        for (const auto& t : other->triangles) {
            auto vo_it = wo_it->flat_begin();
            for (const auto& v : t.flat_iterable()) {
                bvh.insert(v.position, {&v.position, vo_it});
                ++vo_it;
            }
            ++wo_it;
        }
    }
    for (const std::shared_ptr<ColoredVertexArray>& cva : dest.cvas) {
        assert_true(cva->line_bone_weights.empty());
        if (!cva->triangle_bone_weights.empty()) {
            throw std::runtime_error("import_bone_weights requires empty triangle bone weights");
        }
        cva->triangle_bone_weights.reserve(cva->triangles.size());
        for (const auto& t : cva->triangles) {
            FixedArray<std::vector<BoneWeight>, 3> wn;
            std::vector<BoneWeight>* wn_it = wn.flat_begin();
            for (const auto& v : t.flat_iterable()) {
                float best_distance2 = INFINITY;
                const std::vector<BoneWeight>* best_weights = nullptr;
                bvh.visit({v.position, max_distance}, [&](const VertexAndWeights& nv){
                    float dist2 = sum(squared(v.position - *nv.position));
                    if (dist2 < best_distance2) {
                        best_distance2 = dist2;
                        best_weights = nv.weights;
                    }
                    return true;
                });
                if (best_weights == nullptr) {
                    throw std::runtime_error("Could not find weight");
                }
                *wn_it = *best_weights;
                ++wn_it;
            }
            cva->triangle_bone_weights.push_back(wn);
        }
    }
    dest.bone_indices = source.bone_indices;
    dest.skeleton = source.skeleton;
    dest.check_consistency();
}
