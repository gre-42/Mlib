#include "Animated_Colored_Vertex_Arrays.hpp"
#include <Mlib/Assert.hpp>
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Mesh/Barrier_Triangle_Hitbox.hpp>
#include <Mlib/Geometry/Mesh/Bone.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array_Filter.hpp>
#include <Mlib/Geometry/Mesh/Modifiers/Smoothen_Edges.hpp>
#include <Mlib/Geometry/Mesh/Smoothness_Target.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Transformation/Quaternion.hpp>

using namespace Mlib;

AnimatedColoredVertexArrays::AnimatedColoredVertexArrays()
{}

AnimatedColoredVertexArrays::~AnimatedColoredVertexArrays() = default;

UUVector<OffsetAndQuaternion<float, float>> AnimatedColoredVertexArrays::vectorize_joint_poses(
    const std::map<std::string, OffsetAndQuaternion<float, float>>& poses) const
{
    UUVector<OffsetAndQuaternion<float, float>> ms(bone_indices.size());
#ifndef NDEBUG
    for (auto& m : ms) {
        m.t = fixed_nans<float, 3>();
    }
#endif
    for (const auto& [name, pose] : poses) {
        auto it = bone_indices.find(name);
        if (it == bone_indices.end()) {
            THROW_OR_ABORT("vectorize_joint_poses: Could not find bone with name " + name);
        }
        ms.at(it->second) = pose;
    }
#ifndef NDEBUG
    for (const auto& m : ms) {
        if (any(Mlib::isnan(m.t))) {
            THROW_OR_ABORT("Pose contains NAN values or was not set");
        }
    }
#endif
    return ms;
}

std::shared_ptr<AnimatedColoredVertexArrays> AnimatedColoredVertexArrays::generate_grind_lines(
    float edge_angle,
    float averaged_normal_angle,
    const ColoredVertexArrayFilter& filter) const
{
    auto result = std::make_shared<AnimatedColoredVertexArrays>();
    for (const auto& t : scvas) {
        if (filter.matches(*t)) {
            result->scvas.push_back(
                t->generate_grind_lines(edge_angle, averaged_normal_angle));
        }
    }
    for (const auto& t : dcvas) {
        if (filter.matches(*t)) {
            result->dcvas.push_back(
                t->generate_grind_lines(edge_angle, averaged_normal_angle));
        }
    }
    return result;
}

void AnimatedColoredVertexArrays::create_barrier_triangle_hitboxes(
    float depth,
    PhysicsMaterial destination_physics_material,
    const ColoredVertexArrayFilter& filter)
{
    for (const auto& t : scvas) {
        if (filter.matches(*t)) {
            THROW_OR_ABORT("Single-precision array matches terrain convex decomposition filter");
        }
    }
    std::list<std::shared_ptr<ColoredVertexArray<CompressedScenePos>>> new_dvcas;
    for (const auto& cva : dcvas) {
        if (filter.matches(*cva)) {
            for (const auto& cva : Mlib::create_barrier_triangle_hitboxes(
                *cva,
                depth / 2.f,
                destination_physics_material))
            {
                new_dvcas.push_back(cva);
            }
        } else {
            new_dvcas.push_back(cva);
        }
    }
    dcvas = std::move(new_dvcas);
}

void AnimatedColoredVertexArrays::smoothen_edges(
    SmoothnessTarget target,
    float smoothness,
    size_t niterations,
    float decay)
{
    if (target == SmoothnessTarget::PHYSICS) {
        std::list<std::shared_ptr<ColoredVertexArray<CompressedScenePos>>> new_dvcas;
        for (const auto& l : dcvas) {
            if (!any(l->morphology.physics_material & PhysicsMaterial::ATTR_COLLIDE)) {
                continue;
            }
            new_dvcas.emplace_back(std::make_shared<ColoredVertexArray<CompressedScenePos>>(
                l->name + "_smooth",
                l->material,
                l->morphology - PhysicsMaterial::ATTR_VISIBLE,
                l->modifier_backlog,
                UUVector<FixedArray<ColoredVertex<CompressedScenePos>, 4>>{},
                UUVector<FixedArray<ColoredVertex<CompressedScenePos>, 3>>{l->triangles},
                UUVector<FixedArray<ColoredVertex<CompressedScenePos>, 2>>{},
                UUVector<FixedArray<std::vector<BoneWeight>, 3>>{},
                UUVector<FixedArray<float, 3>>{},
                UUVector<FixedArray<uint8_t, 3>>{},
                std::vector<UUVector<FixedArray<float, 3, 2>>>{},
                std::vector<UUVector<FixedArray<float, 3>>>{},
                UUVector<FixedArray<float, 3>>{}));
            l->morphology.physics_material &= ~PhysicsMaterial::ATTR_COLLIDE;
        }
        Mlib::smoothen_edges(new_dvcas, {}, smoothness, niterations, decay);
        dcvas.insert(dcvas.end(), new_dvcas.begin(), new_dvcas.end());
    } else {
        THROW_OR_ABORT("Only physics smoothness-target is implemented");
    }
}

void AnimatedColoredVertexArrays::check_consistency() const {
    assert_true(bone_indices.empty() == !skeleton);
    auto validate_cva = [this](const auto& cvas){
        for (const auto& cva : cvas) {
            assert_true(cva->triangle_bone_weights.empty() == !skeleton);
            if (!cva->triangle_bone_weights.empty()) {
                assert_true(cva->triangle_bone_weights.size() == cva->triangles.size());
            }
            if (!cva->continuous_triangle_texture_layers.empty()) {
                assert_true(cva->continuous_triangle_texture_layers.size() == cva->triangles.size());
            }
            if (!cva->discrete_triangle_texture_layers.empty()) {
                assert_true(cva->discrete_triangle_texture_layers.size() == cva->triangles.size());
            }
        }
    };
    validate_cva(scvas);
    validate_cva(dcvas);
}

template <class TPos>
std::list<std::shared_ptr<ColoredVertexArray<TPos>>>& AnimatedColoredVertexArrays::cvas() {
    if constexpr (std::is_same_v<TPos, float>) {
        return scvas;
    } else if constexpr (std::is_same_v<TPos, CompressedScenePos>) {
        return dcvas;
    } else {
        THROW_OR_ABORT("Unknown mesh precision");
    }
}

void AnimatedColoredVertexArrays::print(std::ostream& ostr) const {
    ostr << "AnimatedColoredVertexArrays\n";
    for (const auto& cva : scvas) {
        cva->print(ostr);
    }
    for (const auto& cva : dcvas) {
        cva->print(ostr);
    }
}

template std::list<std::shared_ptr<ColoredVertexArray<float>>>& AnimatedColoredVertexArrays::cvas();
template std::list<std::shared_ptr<ColoredVertexArray<CompressedScenePos>>>& AnimatedColoredVertexArrays::cvas();
