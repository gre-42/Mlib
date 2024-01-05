#include "Physics_Material.hpp"
#include <Mlib/Json/Base.hpp>
#include <Mlib/Strings/String.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <stdexcept>

using namespace Mlib;

static PhysicsMaterial single_physics_material_from_string(const std::string& s) {
    static const std::map<std::string, PhysicsMaterial> m{
        {"none", PhysicsMaterial::NONE},
        {"attr_visible", PhysicsMaterial::ATTR_VISIBLE},
        {"attr_collide", PhysicsMaterial::ATTR_COLLIDE},
        {"attr_two_sided", PhysicsMaterial::ATTR_TWO_SIDED},
        {"attr_align_strict", PhysicsMaterial::ATTR_ALIGN_STRICT},
        {"attr_convex", PhysicsMaterial::ATTR_CONVEX},
        {"attr_concave", PhysicsMaterial::ATTR_CONCAVE},
        {"attr_round", PhysicsMaterial::ATTR_ROUND},
        {"obj_alignment_plane", PhysicsMaterial::OBJ_ALIGNMENT_PLANE},
        {"obj_chassis", PhysicsMaterial::OBJ_CHASSIS},
        {"obj_tire_line", PhysicsMaterial::OBJ_TIRE_LINE},
        {"obj_grind_contact", PhysicsMaterial::OBJ_GRIND_CONTACT},
        {"obj_grind_line", PhysicsMaterial::OBJ_GRIND_LINE},
        {"obj_alignment_contact", PhysicsMaterial::OBJ_ALIGNMENT_CONTACT},
        {"obj_bullet_line_segment", PhysicsMaterial::OBJ_BULLET_LINE_SEGMENT},
        {"obj_bullet_mesh", PhysicsMaterial::OBJ_BULLET_MESH},
        {"obj_hitbox", PhysicsMaterial::OBJ_HITBOX},
        {"obj_distancebox", PhysicsMaterial::OBJ_DISTANCEBOX},
        {"obj_grass", PhysicsMaterial::OBJ_GRASS},
        {"surface_base_tarmac", PhysicsMaterial::SURFACE_BASE_TARMAC},
        {"surface_base_gravel", PhysicsMaterial::SURFACE_BASE_GRAVEL},
        {"surface_base_snow", PhysicsMaterial::SURFACE_BASE_SNOW},
        {"surface_base_ice", PhysicsMaterial::SURFACE_BASE_ICE},
        {"surface_base_sand", PhysicsMaterial::SURFACE_BASE_SAND},
        {"surface_base_grass", PhysicsMaterial::SURFACE_BASE_GRASS},
        {"surface_base_dirt", PhysicsMaterial::SURFACE_BASE_DIRT},
        {"surface_base_tire", PhysicsMaterial::SURFACE_BASE_TIRE},
        {"surface_base_stone", PhysicsMaterial::SURFACE_BASE_STONE},
        {"surface_base_foliage", PhysicsMaterial::SURFACE_BASE_FOLIAGE},
        {"surface_modifier_wet", PhysicsMaterial::SURFACE_MODIFIER_WET}
    };
    auto it = m.find(s);
    if (it == m.end()) {
        THROW_OR_ABORT("Unknown physics material: \"" + s + '"');
    }
    return it->second;
}

PhysicsMaterial Mlib::physics_material_from_string(const std::string& s) {
    static const DECLARE_REGEX(re, "\\|");
    PhysicsMaterial result = PhysicsMaterial::NONE;
    for (const auto& m : string_to_list(s, re)) {
        result |= single_physics_material_from_string(m);
    }
    return result;
}

std::string Mlib::physics_material_to_string(PhysicsMaterial p) {
    static const std::map<PhysicsMaterial, std::string> m{
        {PhysicsMaterial::NONE, "none"},
        {PhysicsMaterial::SURFACE_BASE_TARMAC, "tarmac"},
        {PhysicsMaterial::SURFACE_BASE_GRAVEL, "gravel"},
        {PhysicsMaterial::SURFACE_BASE_SNOW, "snow"},
        {PhysicsMaterial::SURFACE_BASE_ICE, "ice"},
        {PhysicsMaterial::SURFACE_BASE_SAND, "sand"},
        {PhysicsMaterial::SURFACE_BASE_GRASS, "grass"},
        {PhysicsMaterial::SURFACE_BASE_DIRT, "dirt"},
        {PhysicsMaterial::SURFACE_BASE_TIRE, "tire"},
        {PhysicsMaterial::SURFACE_BASE_STONE, "stone"},
        {PhysicsMaterial::SURFACE_BASE_FOLIAGE, "foliage"}
    };
    auto it = m.find(p);
    if (it == m.end()) {
        return "PhysicsMaterial(" + std::to_string((int)p) + ')';
    }
    return it->second;
}

void Mlib::from_json(const nlohmann::json& j, PhysicsMaterial& p) {
    static const std::map<std::string, PhysicsMaterial> m{
        {"none", PhysicsMaterial::NONE},
        {"tarmac", PhysicsMaterial::SURFACE_BASE_TARMAC},
        {"gravel", PhysicsMaterial::SURFACE_BASE_GRAVEL},
        {"snow", PhysicsMaterial::SURFACE_BASE_SNOW},
        {"ice", PhysicsMaterial::SURFACE_BASE_ICE},
        {"sand", PhysicsMaterial::SURFACE_BASE_SAND},
        {"grass", PhysicsMaterial::SURFACE_BASE_GRASS},
        {"dirt", PhysicsMaterial::SURFACE_BASE_DIRT},
        {"tire", PhysicsMaterial::SURFACE_BASE_TIRE},
        {"stone", PhysicsMaterial::SURFACE_BASE_STONE}
    };
    auto it = m.find(j.get<std::string>());
    if (it == m.end()) {
        THROW_OR_ABORT("Unknown physics material: \"" + j.get<std::string>() + '"');
    }
    p = it->second;
}
