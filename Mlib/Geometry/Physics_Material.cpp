#include "Physics_Material.hpp"
#include <Mlib/Strings/String.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <stdexcept>

using namespace Mlib;

static PhysicsMaterial single_physics_material_from_string(const std::string& s) {
    if (s == "none") {
        return PhysicsMaterial::NONE;
    }
    if (s == "attr_visible") {
        return PhysicsMaterial::ATTR_VISIBLE;
    }
    if (s == "attr_collide") {
        return PhysicsMaterial::ATTR_COLLIDE;
    }
    if (s == "attr_two_sided") {
        return PhysicsMaterial::ATTR_TWO_SIDED;
    }
    if (s == "attr_align_strict") {
        return PhysicsMaterial::ATTR_ALIGN_STRICT;
    }
    if (s == "attr_convex") {
        return PhysicsMaterial::ATTR_CONVEX;
    }
    if (s == "attr_concave") {
        return PhysicsMaterial::ATTR_CONCAVE;
    }
    if (s == "obj_alignment_plane") {
        return PhysicsMaterial::OBJ_ALIGNMENT_PLANE;
    }
    if (s == "obj_chassis") {
        return PhysicsMaterial::OBJ_CHASSIS;
    }
    if (s == "obj_tire_line") {
        return PhysicsMaterial::OBJ_TIRE_LINE;
    }
    if (s == "obj_grind_contact") {
        return PhysicsMaterial::OBJ_GRIND_CONTACT;
    }
    if (s == "obj_grind_line") {
        return PhysicsMaterial::OBJ_GRIND_LINE;
    }
    if (s == "obj_alignment_contact") {
        return PhysicsMaterial::OBJ_ALIGNMENT_CONTACT;
    }
    if (s == "obj_bullet_line_segment") {
        return PhysicsMaterial::OBJ_BULLET_LINE_SEGMENT;
    }
    if (s == "obj_bullet_mesh") {
        return PhysicsMaterial::OBJ_BULLET_MESH;
    }
    if (s == "obj_hitbox") {
        return PhysicsMaterial::OBJ_HITBOX;
    }
    if (s == "obj_distancebox") {
        return PhysicsMaterial::OBJ_DISTANCEBOX;
    }
    if (s == "obj_grass") {
        return PhysicsMaterial::OBJ_GRASS;
    }
    if (s == "surface_base_tarmac") {
        return PhysicsMaterial::SURFACE_BASE_TARMAC;
    }
    if (s == "surface_base_gravel") {
        return PhysicsMaterial::SURFACE_BASE_GRAVEL;
    }
    if (s == "surface_base_snow") {
        return PhysicsMaterial::SURFACE_BASE_SNOW;
    }
    if (s == "surface_base_ice") {
        return PhysicsMaterial::SURFACE_BASE_ICE;
    }
    if (s == "surface_base_sand") {
        return PhysicsMaterial::SURFACE_BASE_SAND;
    }
    if (s == "surface_base_grass") {
        return PhysicsMaterial::SURFACE_BASE_GRASS;
    }
    if (s == "surface_base_dirt") {
        return PhysicsMaterial::SURFACE_BASE_DIRT;
    }
    if (s == "surface_base_tire") {
        return PhysicsMaterial::SURFACE_BASE_TIRE;
    }
    if (s == "surface_base_stone") {
        return PhysicsMaterial::SURFACE_BASE_STONE;
    }
    if (s == "surface_modifier_wet") {
        return PhysicsMaterial::SURFACE_MODIFIER_WET;
    }
    THROW_OR_ABORT("Unknown physics material: \"" + s + '"');
}

PhysicsMaterial Mlib::physics_material_from_string(const std::string& s) {
    static const DECLARE_REGEX(re, "\\|");
    PhysicsMaterial result = PhysicsMaterial::NONE;
    for (const auto& m : string_to_list(s, re)) {
        result |= single_physics_material_from_string(m);
    }
    return result;
}
