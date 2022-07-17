#include "Wing.hpp"
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

Wing::Wing(
    const TransformationMatrix<float, double, 3>& relative_location,
    const Interp<float>& fac,
    float lift_coefficient,
    float angle_coefficient_yz,
    float angle_coefficient_zz,
    const FixedArray<float, 3>& drag_coefficients,
    float angle_of_attack,
    float brake_angle,
    SceneNode* angle_of_attack_scene_node,
    SceneNode* brake_scene_node)
: fac{fac},
  lift_coefficient{lift_coefficient},
  angle_coefficient_yz{angle_coefficient_yz},
  angle_coefficient_zz{angle_coefficient_zz},
  drag_coefficients{drag_coefficients},
  angle_of_attack_{angle_of_attack},
  brake_angle_{brake_angle},
  angle_of_attack_scene_node_{angle_of_attack_scene_node},
  brake_scene_node_{brake_scene_node},
  relative_location_{relative_location}
{}

Wing::~Wing()
{}

TransformationMatrix<float, double, 3> Wing::absolute_location(
    const TransformationMatrix<float, double, 3>& parent_location)
{
    return parent_location * relative_location_;
}

float Wing::angle_of_attack() const {
    return angle_of_attack_;
}

void Wing::set_angle_of_attack(float angle) {
    angle_of_attack_ = angle;
    if (angle_of_attack_scene_node_ != nullptr) {
        angle_of_attack_scene_node_->set_rotation(matrix_2_tait_bryan_angles(rodrigues2(
            relative_location_.R().column(0),
            angle)));
    }
}

float Wing::brake_angle() const {
    return brake_angle_;
}

void Wing::set_brake_angle(float angle) {
    brake_angle_ = angle;
    if (brake_scene_node_ != nullptr) {
        brake_scene_node_->set_rotation(matrix_2_tait_bryan_angles(rodrigues2(
            relative_location_.R().column(0),
            angle)));
    }
}
