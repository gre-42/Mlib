#pragma once
#include <Mlib/Math/Interp.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Physics/Actuators/Trail_Source.hpp>
#include <Mlib/Physics/Actuators/Wing_Angle.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <optional>

namespace Mlib {

class ITrailExtender;

class Wing {
    Wing(const Wing&) = delete;
    Wing& operator = (const Wing&) = delete;
public:
    Wing(
        DanglingPtr<SceneNode> angle_of_attack_node,
        DanglingPtr<SceneNode> brake_angle_node,
        const TransformationMatrix<float, ScenePos, 3>& relative_location,
        const Interp<float>& fac,
        float lift_coefficient,
        float angle_coefficient_yz,
        float angle_coefficient_zz,
        const FixedArray<float, 3>& drag_coefficients,
        float angle_of_attack,
        float brake_angle,
        std::optional<TrailSource> trail_source);
    ~Wing();
    TransformationMatrix<float, ScenePos, 3> absolute_location(
        const TransformationMatrix<float, ScenePos, 3>& parent_location) const;
    Interp<float> fac;
    float lift_coefficient;
    float angle_coefficient_yz;
    float angle_coefficient_zz;
    FixedArray<float, 3> drag_coefficients;
    float angle_of_attack;
    float brake_angle;
    WingAngle angle_of_attack_movable;
    WingAngle brake_angle_movable;
    std::optional<TrailSource> trail_source;

private:
    TransformationMatrix<float, ScenePos, 3> relative_location_;
};

}
