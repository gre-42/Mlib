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
        {"attr_contains_skidmarks", PhysicsMaterial::ATTR_CONTAINS_SKIDMARKS},
        {"attr_slippery", PhysicsMaterial::ATTR_SLIPPERY},
        {"attr_animated_textures", PhysicsMaterial::ATTR_ANIMATED_TEXTURES},
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
        {"surface_base_metal", PhysicsMaterial::SURFACE_BASE_METAL},
        {"surface_base_concrete", PhysicsMaterial::SURFACE_BASE_CONCRETE},
        {"surface_base_glass", PhysicsMaterial::SURFACE_BASE_GLASS},
        {"surface_base_water", PhysicsMaterial::SURFACE_BASE_WATER},
        {"surface_wet", PhysicsMaterial::SURFACE_WET}
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

static std::string physics_material_modifiers_to_string(PhysicsMaterial p) {
    static const std::map<PhysicsMaterial, std::string> m{
        { PhysicsMaterial::ATTR_VISIBLE, "attr_visible" },
        { PhysicsMaterial::ATTR_COLLIDE, "attr_collide" },
        { PhysicsMaterial::ATTR_TWO_SIDED, "attr_two_sided" },
        { PhysicsMaterial::ATTR_ALIGN_STRICT, "attr_align_strict" },
        { PhysicsMaterial::ATTR_CONVEX, "attr_convex" },
        { PhysicsMaterial::ATTR_CONCAVE, "attr_concave" },
        { PhysicsMaterial::ATTR_ROUND, "attr_round" },
        { PhysicsMaterial::ATTR_CONTAINS_SKIDMARKS, "attr_contains_skidmarks" },
        { PhysicsMaterial::ATTR_SLIPPERY, "attr_slippery" },
        { PhysicsMaterial::ATTR_ANIMATED_TEXTURES, "attr_animated_textures" },
        { PhysicsMaterial::ATTR_LIQUID, "attr_liquid" },
        { PhysicsMaterial::OBJ_ALIGNMENT_PLANE, "obj_alignment_plane" },
        { PhysicsMaterial::OBJ_CHASSIS, "obj_chassis" },
        { PhysicsMaterial::OBJ_TIRE_LINE, "obj_tire_line" },
        { PhysicsMaterial::OBJ_GRIND_CONTACT, "obj_grind_contact" },
        { PhysicsMaterial::OBJ_GRIND_LINE, "obj_grind_line" },
        { PhysicsMaterial::OBJ_ALIGNMENT_CONTACT, "obj_alignment_contact" },
        { PhysicsMaterial::OBJ_BULLET_LINE_SEGMENT, "obj_bullet_line_segment" },
        { PhysicsMaterial::OBJ_BULLET_MESH, "obj_bullet_mesh" },
        { PhysicsMaterial::OBJ_HITBOX, "obj_hitbox" },
        { PhysicsMaterial::OBJ_DISTANCEBOX, "obj_distancebox" },
        { PhysicsMaterial::OBJ_GRASS, "obj_grass" },
        { PhysicsMaterial::SURFACE_WET, "surface_wet" }
    };
    std::string result;
    for (auto& [modifier, str] : m) {
        if (any(modifier & p)) {
            if (!result.empty()) {
                result += '|';
            }
            result += str;
            p &= ~modifier;
        }
    }
    p &= ~PhysicsMaterial::SURFACE_BASE_MASK;
    if (any(p)) {
        THROW_OR_ABORT("Unknown modifiers in physics material: " + std::to_string((int)p));
    }
    return result;
}

static std::string physics_material_base_to_string(PhysicsMaterial p) {
    switch (p & PhysicsMaterial::SURFACE_BASE_MASK) {
        case PhysicsMaterial::NONE: return "none";
        case PhysicsMaterial::SURFACE_BASE_TARMAC: return "surface_base_tarmac";
        case PhysicsMaterial::SURFACE_BASE_GRAVEL: return "surface_base_gravel";
        case PhysicsMaterial::SURFACE_BASE_SNOW: return "surface_base_snow";
        case PhysicsMaterial::SURFACE_BASE_ICE: return "surface_base_ice";
        case PhysicsMaterial::SURFACE_BASE_SAND: return "surface_base_sand";
        case PhysicsMaterial::SURFACE_BASE_GRASS: return "surface_base_grass";
        case PhysicsMaterial::SURFACE_BASE_DIRT: return "surface_base_dirt";
        case PhysicsMaterial::SURFACE_BASE_TIRE: return "surface_base_tire";
        case PhysicsMaterial::SURFACE_BASE_STONE: return "surface_base_stone";
        case PhysicsMaterial::SURFACE_BASE_FOLIAGE: return "surface_base_foliage";
        case PhysicsMaterial::SURFACE_BASE_METAL: return "surface_base_metal";
        case PhysicsMaterial::SURFACE_BASE_CONCRETE: return "surface_base_concrete";
        case PhysicsMaterial::SURFACE_BASE_GLASS: return "surface_base_glass";
        case PhysicsMaterial::SURFACE_BASE_WATER: return "surface_base_water";
        default: return "PhysicsMaterial(" + std::to_string((uint32_t)p) + ')';
    };
}

std::string Mlib::physics_material_to_string(PhysicsMaterial p) {
    auto modifiers = physics_material_modifiers_to_string(p);
    auto base = physics_material_base_to_string(p);
    if (!modifiers.empty() && !base.empty()) {
        return modifiers + '|' + base;
    } else {
        return modifiers + base;
    }
}

void Mlib::from_json(const nlohmann::json& j, PhysicsMaterial& p) {
    p = physics_material_from_string(j.get<std::string>());
}
