#include "Animated_Colored_Vertex_Arrays.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Mesh/Bone.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array_Filter.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Quaternion.hpp>

using namespace Mlib;

AnimatedColoredVertexArrays::AnimatedColoredVertexArrays()
{}

AnimatedColoredVertexArrays::~AnimatedColoredVertexArrays()
{}

std::vector<OffsetAndQuaternion<float, float>> AnimatedColoredVertexArrays::vectorize_joint_poses(
    const std::map<std::string, OffsetAndQuaternion<float, float>>& poses) const
{
    std::vector<OffsetAndQuaternion<float, float>> ms(bone_indices.size());
#ifndef NDEBUG
    for (auto& m : ms) {
        m.offset() = fixed_nans<float, 3>();
    }
#endif
    for (const auto& p : poses) {
        auto it = bone_indices.find(p.first);
        if (it == bone_indices.end()) {
            THROW_OR_ABORT("vectorize_joint_poses: Could not find bone with name " + p.first);
        }
        ms.at(it->second) = p.second;
    }
#ifndef NDEBUG
    for (const auto& m : ms) {
        if (any(Mlib::isnan(m.offset()))) {
            THROW_OR_ABORT("Pose contains NAN values or was not set");
        }
    }
#endif
    return ms;
}

std::shared_ptr<AnimatedColoredVertexArrays> AnimatedColoredVertexArrays::generate_grind_lines(
    float edge_angle,
    float averaged_normal_angle,
    const ColoredVertexArrayFilter& filter)
{
    auto result = std::make_shared<AnimatedColoredVertexArrays>();
    for (auto& t : scvas) {
        if (filter.matches(*t)) {
            result->scvas.push_back(std::make_shared<ColoredVertexArray<float>>(
                t->generate_grind_lines(edge_angle, averaged_normal_angle)));
        }
    }
    for (auto& t : dcvas) {
        if (filter.matches(*t)) {
            result->dcvas.push_back(std::make_shared<ColoredVertexArray<double>>(
                t->generate_grind_lines(edge_angle, averaged_normal_angle)));
        }
    }
    return result;
}

void AnimatedColoredVertexArrays::check_consistency() const {
    assert_true(bone_indices.empty() == !skeleton);
    auto validate_cva = [this](const auto& cvas){
        for (const auto& cva : cvas) {
            assert_true(cva->triangle_bone_weights.empty() == !skeleton);
            // assert_true(cva->line_bone_weights.empty() == !skeleton);
            assert_true(cva->line_bone_weights.empty());
            if (!cva->triangle_bone_weights.empty()) {
                assert_true(cva->triangle_bone_weights.size() == cva->triangles.size());
            }
            if (!cva->line_bone_weights.empty()) {
                assert_true(cva->line_bone_weights.size() == cva->line_bone_weights.size());
            }
        }
    };
    validate_cva(scvas);
    validate_cva(dcvas);
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
