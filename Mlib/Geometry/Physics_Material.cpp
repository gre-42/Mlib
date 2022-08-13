#include "Physics_Material.hpp"
#include <stdexcept>

using namespace Mlib;

PhysicsMaterial Mlib::physics_material_from_string(const std::string& s) {
    if (s == "none") {
        return PhysicsMaterial::NONE;
    } else if (s == "attr_visible") {
        return PhysicsMaterial::ATTR_VISIBLE;
    } else if (s == "attr_collide") {
        return PhysicsMaterial::ATTR_COLLIDE;
    } else if (s == "attr_two_sided") {
        return PhysicsMaterial::ATTR_TWO_SIDED;
    } else if (s == "attr_align_strict") {
        return PhysicsMaterial::ATTR_ALIGN_STRICT;
    } else if (s == "obj_alignment_plane") {
        return PhysicsMaterial::OBJ_ALIGNMENT_PLANE;
    } else if (s == "obj_chassis") {
        return PhysicsMaterial::OBJ_CHASSIS;
    } else if (s == "obj_tire_line") {
        return PhysicsMaterial::OBJ_TIRE_LINE;
    } else if (s == "obj_grind_contact") {
        return PhysicsMaterial::OBJ_GRIND_CONTACT;
    } else if (s == "obj_grind_line") {
        return PhysicsMaterial::OBJ_GRIND_LINE;
    } else if (s == "obj_alignment_contact") {
        return PhysicsMaterial::OBJ_ALIGNMENT_CONTACT;
    } else if (s == "obj_bullet_line_segment") {
        return PhysicsMaterial::OBJ_BULLET_LINE_SEGMENT;
    } else if (s == "obj_bullet_mesh") {
        return PhysicsMaterial::OBJ_BULLET_MESH;
    } else if (s == "obj_hitbox") {
        return PhysicsMaterial::OBJ_HITBOX;
    } else if (s == "obj_distancebox") {
        return PhysicsMaterial::OBJ_DISTANCEBOX;
    } else {
        throw std::runtime_error("Unknown physics material: \"" + s + '"');
    }
}
