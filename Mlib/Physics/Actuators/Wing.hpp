#pragma once
#include <Mlib/Math/Interp.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>

namespace Mlib {

class SceneNode;

class Wing {
public:
    Wing(
        const TransformationMatrix<float, double, 3>& relative_location,
        const Interp<float>& fac,
        float lift_coefficient,
        float angle_coefficient_yz,
        float angle_coefficient_zz,
        const FixedArray<float, 3>& drag_coefficients,
        float angle_of_attack,
        float brake_angle,
        SceneNode* angle_of_attack_scene_node,
        SceneNode* brake_scene_node);
    Wing(const Wing&) = delete;
    Wing& operator = (const Wing&) = delete;
    ~Wing();
    TransformationMatrix<float, double, 3> absolute_location(
        const TransformationMatrix<float, double, 3>& parent_location);
    float angle_of_attack() const;
    void set_angle_of_attack(float angle);
    float brake_angle() const;
    void set_brake_angle(float angle);
    Interp<float> fac;
    float lift_coefficient;
    float angle_coefficient_yz;
    float angle_coefficient_zz;
    FixedArray<float, 3> drag_coefficients;
private:
    float angle_of_attack_;
    float brake_angle_;
    SceneNode* angle_of_attack_scene_node_;
    SceneNode* brake_scene_node_;
    TransformationMatrix<float, double, 3> relative_location_;
};

}
