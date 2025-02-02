#include "Wing.hpp"
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Physics/Actuators/Trail_Source.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Interfaces/ITrail_Extender.hpp>

using namespace Mlib;

Wing::Wing(
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
    std::optional<TrailSource> trail_source)
    : fac{ fac }
    , lift_coefficient{ lift_coefficient }
    , angle_coefficient_yz{ angle_coefficient_yz }
    , angle_coefficient_zz{ angle_coefficient_zz }
    , drag_coefficients{ drag_coefficients }
    , angle_of_attack{ angle_of_attack }
    , brake_angle{ brake_angle }
    , angle_of_attack_movable{ angle_of_attack_node, this->angle_of_attack, relative_location.R.column(0) }
    , brake_angle_movable{ brake_angle_node, this->brake_angle, relative_location.R.column(0) }
    , trail_source{ std::move(trail_source) }
    , relative_location_{ relative_location }
{}

Wing::~Wing() = default;

TransformationMatrix<float, ScenePos, 3> Wing::absolute_location(
    const TransformationMatrix<float, ScenePos, 3>& parent_location) const
{
    return parent_location * relative_location_;
}
