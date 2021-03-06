#include "Osm_Triangle_Lists.hpp"
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Entrance_Type.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Resource_Config.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Road_Type.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Styled_Road.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Terrain_Type.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Water_Type.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>

using namespace Mlib;

static const float ROAD_SPECULARITY = 0.2f;

void RoadPropertiesTriangleList::append(const StyledRoadEntry& entry) {
    lst_.push_back(entry);
}

const StyledRoad& RoadPropertiesTriangleList::operator [] (const RoadProperties& road_properties) const {
    const StyledRoad* result = nullptr;
    for (const auto& l : lst_) {
        if (l.road_properties.type != road_properties.type) {
            continue;
        }
        if (l.road_properties.nlanes <= road_properties.nlanes) {
            result = &l.styled_road;
        }
    }
    if (result == nullptr) {
        throw std::runtime_error("Could not find matching triangle list for properties " + (std::string)road_properties);
    }
    return *result;
}

const std::list<StyledRoadEntry>& RoadPropertiesTriangleList::list() const {
    return lst_;
}

template <class EntityType>
void EntityTypeTriangleList<EntityType>::insert(EntityType road_type, const std::shared_ptr<TriangleList<double>>& lst) {
    if (!lst_.insert({road_type, lst}).second) {
        throw std::runtime_error("Could not insert triangle list");
    }
}

template <class EntityType>
bool EntityTypeTriangleList<EntityType>::contains(EntityType road_type) const {
    return lst_.find(road_type) != lst_.end();
}

template <class EntityType>
const std::shared_ptr<TriangleList<double>>& EntityTypeTriangleList<EntityType>::operator [] (EntityType road_type) const {
    auto it = lst_.find(road_type);
    if (it == lst_.end()) {
        throw std::runtime_error("Could not find list with type " + to_string(road_type));
    }
    return it->second;
}

template <class EntityType>
const std::map<EntityType, std::shared_ptr<TriangleList<double>>>& EntityTypeTriangleList<EntityType>::map() const {
    return lst_;
}

OsmTriangleLists::OsmTriangleLists(
    const OsmResourceConfig& config,
    const std::string& name_suffix)
{
    auto primary_rendering_resources = RenderingContextStack::primary_rendering_resources();
    tl_terrain = std::make_shared<TerrainTypeTriangleList>();
    tl_terrain->insert(
        TerrainType::STREET_HOLE,
        std::make_shared<TriangleList<double>>(
            terrain_type_to_string(TerrainType::STREET_HOLE) + name_suffix,
            Material(),
            PhysicsMaterial::ATTR_VISIBLE | PhysicsMaterial::ATTR_COLLIDE));
    tl_terrain->insert(
        TerrainType::BUILDING_HOLE,
        std::make_shared<TriangleList<double>>(terrain_type_to_string(TerrainType::BUILDING_HOLE) + name_suffix,
        Material(),
        PhysicsMaterial::ATTR_VISIBLE | PhysicsMaterial::ATTR_COLLIDE));
    for (auto& ttt : config.terrain_textures) {
        auto dt = config.terrain_dirt_textures.find(ttt.first);
        std::string dirt_texture = (dt == config.terrain_dirt_textures.end()) ? "" : dt->second;
        tl_terrain->insert(ttt.first, std::make_shared<TriangleList<double>>(
            terrain_type_to_string(ttt.first) + name_suffix,
            Material{
                .dirt_texture = dirt_texture,
                .occluded_pass = ExternalRenderPassType::LIGHTMAP_BLACK_NODE,
                .occluder_pass = ExternalRenderPassType::NONE,
                .aggregate_mode = AggregateMode::ONCE,
                .specularity = {0.f, 0.f, 0.f},
                .draw_distance_noperations = 1000}.compute_color_mode(),
            PhysicsMaterial::ATTR_VISIBLE | PhysicsMaterial::ATTR_COLLIDE));
        tl_terrain_visuals.insert(ttt.first, std::make_shared<TriangleList<double>>(
            terrain_type_to_string(ttt.first) + "_visuals" + name_suffix,
            Material{
                .dirt_texture = dirt_texture,
                .occluded_pass = ExternalRenderPassType::LIGHTMAP_BLACK_NODE,
                .occluder_pass = ExternalRenderPassType::NONE,
                .aggregate_mode = AggregateMode::ONCE,
                .specularity = {0.f, 0.f, 0.f},
                .draw_distance_noperations = 1000}.compute_color_mode(),
            PhysicsMaterial::ATTR_VISIBLE));
        tl_terrain_extrusion.insert(ttt.first, std::make_shared<TriangleList<double>>(
            terrain_type_to_string(ttt.first) + "_street_extrusion" + name_suffix,
            Material{
                .dirt_texture = dirt_texture,
                .occluded_pass = ExternalRenderPassType::LIGHTMAP_BLACK_NODE,
                .occluder_pass = ExternalRenderPassType::NONE,
                .aggregate_mode = AggregateMode::ONCE,
                .specularity = {0.f, 0.f, 0.f},
                .draw_distance_noperations = 1000}.compute_color_mode(),
            PhysicsMaterial::ATTR_VISIBLE | PhysicsMaterial::ATTR_COLLIDE));
        for (auto& t : ttt.second) {
            // BlendMapTexture bt{ .texture_descriptor = {.color = t, .normal = primary_rendering_resources->get_normalmap(t), .anisotropic_filtering_level = anisotropic_filtering_level } };
            BlendMapTexture bt = primary_rendering_resources->get_blend_map_texture(t);
            (*tl_terrain)[ttt.first]->material_.textures.push_back(bt);
            tl_terrain_visuals[ttt.first]->material_.textures.push_back(bt);
            tl_terrain_extrusion[ttt.first]->material_.textures.push_back(bt);
        }
        (*tl_terrain)[ttt.first]->material_.compute_color_mode();
        tl_terrain_visuals[ttt.first]->material_.compute_color_mode();
        tl_terrain_extrusion[ttt.first]->material_.compute_color_mode();
    }
    for (const auto& s : config.street_crossing_texture) {
        tl_street_crossing.insert(s.first, std::make_shared<TriangleList<double>>(
            "crossing_" + road_type_to_string(s.first) + name_suffix,
            Material{
                .textures = {primary_rendering_resources->get_blend_map_texture(s.second)},
                .dirt_texture = config.street_dirt_texture,
                .occluded_pass = (s.first != RoadType::WALL) ? ExternalRenderPassType::LIGHTMAP_BLACK_NODE : ExternalRenderPassType::NONE,
                .occluder_pass = (s.first != RoadType::WALL) ? ExternalRenderPassType::NONE : ExternalRenderPassType::LIGHTMAP_BLACK_NODE,
                .aggregate_mode = AggregateMode::ONCE,
                .specularity = OrderableFixedArray<float, 3>{ROAD_SPECULARITY * fixed_full<float, 3>((float)(s.first != RoadType::WALL))},
                .draw_distance_noperations = 1000}.compute_color_mode(),
            PhysicsMaterial::ATTR_VISIBLE | PhysicsMaterial::ATTR_COLLIDE));
    }
    for (const auto& s : config.street_texture) {
        bool blend = config.blend_street && (s.first.type != RoadType::WALL);
        std::vector<BlendMapTexture> textures;
        textures.reserve(s.second.textures.size());
        for (const std::string& texture : s.second.textures) {
            textures.push_back(primary_rendering_resources->get_blend_map_texture(texture));
        }
        tl_street.append(StyledRoadEntry{
            .road_properties = s.first,
            .styled_road = StyledRoad{
                .triangle_list = std::make_shared<TriangleList<double>>(
                    (std::string)s.first + name_suffix,
                    Material{
                        .blend_mode = blend ? BlendMode::CONTINUOUS : BlendMode::OFF,
                        .depth_func = blend ? DepthFunc::EQUAL : DepthFunc::LESS,
                        .textures = textures,
                        .reflection_map = config.street_reflection_map,
                        .dirt_texture = config.street_dirt_texture,
                        .occluded_pass = (s.first.type != RoadType::WALL) ? ExternalRenderPassType::LIGHTMAP_BLACK_NODE : ExternalRenderPassType::NONE,
                        .occluder_pass = (s.first.type != RoadType::WALL) ? ExternalRenderPassType::NONE : ExternalRenderPassType::LIGHTMAP_BLACK_NODE,
                        // depth-func==equal requires aggregation, because the terrain is also aggregated.
                        .aggregate_mode = AggregateMode::ONCE,
                        .specularity = OrderableFixedArray<float, 3>{0.f * ROAD_SPECULARITY * fixed_full<float, 3>((float)(s.first.type != RoadType::WALL))},
                        .reflect_only_y = true,
                        .draw_distance_noperations = 1000}.compute_color_mode(),
                    PhysicsMaterial::ATTR_VISIBLE | PhysicsMaterial::ATTR_COLLIDE),
                .uvx = s.second.uvx}}); // mixed_texture: terrain_texture
    }
    WrapMode curb_wrap_mode_s = (config.extrude_curb_amount != 0) || ((config.curb_alpha != 1) && (config.extrude_street_amount != 0)) ? WrapMode::REPEAT : WrapMode::CLAMP_TO_EDGE;
    for (const auto& s : config.curb_street_texture) {
        tl_street_curb.insert(s.first, std::make_shared<TriangleList<double>>(
            "curb_" + road_type_to_string(s.first) + name_suffix,
            Material{
                .textures = {primary_rendering_resources->get_blend_map_texture(s.second)},
                .occluded_pass = (s.first != RoadType::WALL) ? ExternalRenderPassType::LIGHTMAP_BLACK_NODE : ExternalRenderPassType::NONE,
                .occluder_pass = (s.first != RoadType::WALL) ? ExternalRenderPassType::NONE : ExternalRenderPassType::LIGHTMAP_BLACK_NODE,
                .wrap_mode_s = curb_wrap_mode_s,
                .aggregate_mode = AggregateMode::ONCE,
                .specularity = OrderableFixedArray<float, 3>{ROAD_SPECULARITY * fixed_full<float, 3>((float)(config.extrude_curb_amount == 0 && s.first != RoadType::WALL))},
                .draw_distance_noperations = 1000}.compute_color_mode(),
            PhysicsMaterial::ATTR_VISIBLE | PhysicsMaterial::ATTR_COLLIDE)); // mixed_texture: terrain_texture
    }
    for (const auto& s : config.curb2_street_texture) {
        tl_street_curb2.insert(s.first, std::make_shared<TriangleList<double>>(
            "curb2_" + road_type_to_string(s.first) + name_suffix,
            Material{
                .textures = {primary_rendering_resources->get_blend_map_texture(s.second)},
                .occluded_pass = (s.first != RoadType::WALL) ? ExternalRenderPassType::LIGHTMAP_BLACK_NODE : ExternalRenderPassType::NONE,
                .occluder_pass = (s.first != RoadType::WALL) ? ExternalRenderPassType::NONE : ExternalRenderPassType::LIGHTMAP_BLACK_NODE,
                .aggregate_mode = AggregateMode::ONCE,
                .specularity = OrderableFixedArray<float, 3>{ROAD_SPECULARITY * fixed_full<float, 3>((float)(s.first != RoadType::WALL))},
                .draw_distance_noperations = 1000}.compute_color_mode(),
            PhysicsMaterial::ATTR_VISIBLE | PhysicsMaterial::ATTR_COLLIDE)); // mixed_texture: terrain_texture
    }
    for (const auto& s : config.air_curb_street_texture) {
        tl_air_street_curb.insert(s.first, std::make_shared<TriangleList<double>>(
            "air_curb_" + road_type_to_string(s.first) + name_suffix,
            Material{
                .textures = {primary_rendering_resources->get_blend_map_texture(s.second)},
                .occluded_pass = ExternalRenderPassType::LIGHTMAP_BLACK_NODE,
                .occluder_pass = ExternalRenderPassType::NONE,
                .wrap_mode_s = curb_wrap_mode_s,
                .aggregate_mode = AggregateMode::ONCE,
                .draw_distance_noperations = 1000}.compute_color_mode(),
            PhysicsMaterial::ATTR_VISIBLE | PhysicsMaterial::ATTR_COLLIDE));
    }
    tl_racing_line = std::make_shared<TriangleList<double>>(
        "racing_line" + name_suffix,
        Material{
            .blend_mode = BlendMode::CONTINUOUS,
            .continuous_blending_z_order = 1,
            .depth_func = DepthFunc::EQUAL,
            .textures = {primary_rendering_resources->get_blend_map_texture(config.racing_line_texture)},
            .wrap_mode_s = WrapMode::CLAMP_TO_BORDER,
            .wrap_mode_t = WrapMode::REPEAT,
            // depth-func==equal requires aggregation, because the terrain is also aggregated.
            .aggregate_mode = AggregateMode::ONCE,
            .specularity = {0.f, 0.f, 0.f},
            .draw_distance_noperations = 1000}.compute_color_mode(),
        PhysicsMaterial::ATTR_VISIBLE);
    tl_ditch = std::make_shared<TriangleList<double>>("ditch" + name_suffix, Material(), PhysicsMaterial::ATTR_COLLIDE);
    tl_air_support = std::make_shared<TriangleList<double>>(
        "air_support" + name_suffix,
        Material{
            .textures = {primary_rendering_resources->get_blend_map_texture(config.air_support_texture)},
            .occluded_pass = ExternalRenderPassType::LIGHTMAP_BLACK_NODE,
            .occluder_pass = ExternalRenderPassType::NONE,
            .aggregate_mode = AggregateMode::ONCE,
            .draw_distance_noperations = 1000}.compute_color_mode(),
        PhysicsMaterial::ATTR_VISIBLE | PhysicsMaterial::ATTR_COLLIDE);
    tl_tunnel_crossing = std::make_shared<TriangleList<double>>(
        "tunnel_crossing" + name_suffix,
        Material{
            .textures = {primary_rendering_resources->get_blend_map_texture(config.tunnel_pipe_texture)},
            .occluded_pass = ExternalRenderPassType::LIGHTMAP_BLACK_NODE,
            .occluder_pass = ExternalRenderPassType::NONE,
            .aggregate_mode = AggregateMode::ONCE,
            .specularity = {0.f, 0.f, 0.f},
            .draw_distance_noperations = 1000}.compute_color_mode(),
        PhysicsMaterial::ATTR_VISIBLE | PhysicsMaterial::ATTR_COLLIDE);
    tl_tunnel_pipe = std::make_shared<TriangleList<double>>(
        "tunnel_pipe" + name_suffix,
        Material{
            .textures = {primary_rendering_resources->get_blend_map_texture(config.tunnel_pipe_texture)},
            .occluded_pass = ExternalRenderPassType::LIGHTMAP_BLACK_NODE,
            .occluder_pass = ExternalRenderPassType::NONE,
            .aggregate_mode = AggregateMode::ONCE,
            .specularity = {0.f, 0.f, 0.f},
            .draw_distance_noperations = 1000}.compute_color_mode(),
        PhysicsMaterial::ATTR_VISIBLE | PhysicsMaterial::ATTR_COLLIDE);
    tl_tunnel_bdry = std::make_shared<TriangleList<double>>(
        "tunnel_bdry" + name_suffix,
        Material(),
        PhysicsMaterial::ATTR_VISIBLE | PhysicsMaterial::ATTR_COLLIDE);
    tl_entrance[EntranceType::TUNNEL] = std::make_shared<TriangleList<double>>(
        "tunnel_entrance" + name_suffix,
        Material(),
        PhysicsMaterial::ATTR_VISIBLE | PhysicsMaterial::ATTR_COLLIDE);
    tl_entrance[EntranceType::BRIDGE] = std::make_shared<TriangleList<double>>(
        "bridge_entrance" + name_suffix,
        Material(),
        PhysicsMaterial::ATTR_VISIBLE | PhysicsMaterial::ATTR_COLLIDE);
    entrances[EntranceType::TUNNEL];
    entrances[EntranceType::BRIDGE];
    tl_water.insert(WaterType::UNDEFINED, std::make_shared<TriangleList<double>>(
        "water" + name_suffix,
        Material{
            .textures = {primary_rendering_resources->get_blend_map_texture(config.water_texture)},
            .aggregate_mode = AggregateMode::ONCE,
            .draw_distance_noperations = 1000}.compute_color_mode(),
        PhysicsMaterial::ATTR_VISIBLE));
}

OsmTriangleLists::~OsmTriangleLists()
{}

#define INSERT(a) a->triangles_.insert(a->triangles_.end(), other.a->triangles_.begin(), other.a->triangles_.end())
#define INSERT2(a) for (const auto& s : other.a.map()) a[s.first]->triangles_.insert(a[s.first]->triangles_.end(), s.second->triangles_.begin(), s.second->triangles_.end())
#define INSERT2p(a) for (const auto& s : other.a->map()) (*a)[s.first]->triangles_.insert((*a)[s.first]->triangles_.end(), s.second->triangles_.begin(), s.second->triangles_.end())
#define INSERT3(a) for (const auto& s : other.a.list()) a.append(s);
void OsmTriangleLists::insert(const OsmTriangleLists& other) {
    INSERT2p(tl_terrain);
    INSERT2(tl_terrain_visuals);
    INSERT2(tl_terrain_extrusion);
    INSERT2(tl_street_crossing);
    INSERT3(tl_street);
    INSERT2(tl_street_curb);
    INSERT2(tl_street_curb2);
    INSERT2(tl_air_street_curb);
    INSERT(tl_air_support);
    INSERT(tl_tunnel_crossing);
    INSERT(tl_tunnel_pipe);
    INSERT(tl_tunnel_bdry);
}
#undef INSERT
#undef INSERT2
#undef INSERT2p
#undef INSERT3

std::list<std::shared_ptr<TriangleList<double>>> OsmTriangleLists::tls_street_wo_curb() const {
    auto res = std::list<std::shared_ptr<TriangleList<double>>>{};
    for (const auto& e : tl_street_crossing.map()) {if (e.first != RoadType::WALL) res.push_back(e.second);}
    for (const auto& e : tl_street.list()) {if (e.road_properties.type != RoadType::WALL) res.push_back(e.styled_road.triangle_list);}
    return res;
}

std::list<std::shared_ptr<TriangleList<double>>> OsmTriangleLists::tls_street_wo_curb_follower() const {
    auto res = std::list<std::shared_ptr<TriangleList<double>>>{
        tl_racing_line};
    return res;
}

std::list<std::shared_ptr<TriangleList<double>>> OsmTriangleLists::tls_wall_wo_curb() const {
    auto res = std::list<std::shared_ptr<TriangleList<double>>>{};
    for (const auto& e : tl_street_crossing.map()) {if (e.first == RoadType::WALL) res.push_back(e.second);}
    for (const auto& e : tl_street.list()) {if (e.road_properties.type == RoadType::WALL) res.push_back(e.styled_road.triangle_list);}
    return res;
}

std::list<std::shared_ptr<TriangleList<double>>> OsmTriangleLists::tls_street() const {
    auto res = std::list<std::shared_ptr<TriangleList<double>>>{
        tl_air_support};
    for (const auto& e : tl_street_crossing.map()) {res.push_back(e.second);}
    for (const auto& e : tl_street.list()) {res.push_back(e.styled_road.triangle_list);}
    for (const auto& e : tl_street_curb.map()) {res.push_back(e.second);}
    for (const auto& e : tl_street_curb2.map()) {res.push_back(e.second);}
    for (const auto& e : tl_air_street_curb.map()) {res.push_back(e.second);}
    return res;
}

std::list<std::shared_ptr<TriangleList<double>>> OsmTriangleLists::tls_terrain_nosmooth() const {
    auto res = std::list<std::shared_ptr<TriangleList<double>>>{
        tl_air_support,
        tl_ditch};
    for (const auto& e : tl_street_crossing.map()) {res.push_back(e.second);}
    for (const auto& e : tl_street.list()) {res.push_back(e.styled_road.triangle_list);}
    for (const auto& e : tl_street_curb.map()) {res.push_back(e.second);}
    for (const auto& e : tl_street_curb2.map()) {res.push_back(e.second);}
    for (const auto& e : tl_air_street_curb.map()) {res.push_back(e.second);}
    return res;
}

std::list<std::shared_ptr<TriangleList<double>>> OsmTriangleLists::tls_smooth() const {
    auto res = std::list<std::shared_ptr<TriangleList<double>>>{};
    for (const auto& e : tl_terrain->map()) {res.push_back(e.second);}
    for (const auto& e : tl_terrain_visuals.map()) {res.push_back(e.second);}
    // for (const auto& e : tl_terrain_extrusion.map()) {res.push_back(e.second);}
    for (const auto& e : tl_street_crossing.map()) {res.push_back(e.second);}
    for (const auto& e : tl_street.list()) {res.push_back(e.styled_road.triangle_list);}
    for (const auto& e : tl_street_curb.map()) {res.push_back(e.second);}
    for (const auto& e : tl_street_curb2.map()) {res.push_back(e.second);}
    for (const auto& e : tl_air_street_curb.map()) {res.push_back(e.second);}
    return res;
}

std::list<std::shared_ptr<TriangleList<double>>> OsmTriangleLists::tls_no_backfaces() const {
    auto res = std::list<std::shared_ptr<TriangleList<double>>>{
        tl_air_support,
        tl_tunnel_crossing,
        tl_racing_line};
    for (const auto& e : tl_terrain->map()) {res.push_back(e.second);}
    for (const auto& e : tl_terrain_visuals.map()) {res.push_back(e.second);}
    // for (const auto& e : tl_terrain_extrusion.map()) {res.push_back(e.second);}
    for (const auto& e : tl_street_crossing.map()) {res.push_back(e.second);}
    for (const auto& e : tl_street.list()) {res.push_back(e.styled_road.triangle_list);}
    for (const auto& e : tl_street_curb.map()) {res.push_back(e.second);}
    for (const auto& e : tl_street_curb2.map()) {res.push_back(e.second);}
    for (const auto& e : tl_air_street_curb.map()) {res.push_back(e.second);}
    return res;
}

std::list<std::shared_ptr<TriangleList<double>>> OsmTriangleLists::tls_wo_subtraction_and_water() const {
    auto res = std::list<std::shared_ptr<TriangleList<double>>>{
        tl_air_support,
        tl_tunnel_crossing,
        tl_tunnel_pipe,
        tl_racing_line};
    for (const auto& e : tl_terrain->map()) {if (e.first != TerrainType::BUILDING_HOLE) res.push_back(e.second);}
    for (const auto& e : tl_terrain_visuals.map()) {res.push_back(e.second);}
    for (const auto& e : tl_terrain_extrusion.map()) {res.push_back(e.second);}
    for (const auto& e : tl_street_crossing.map()) {res.push_back(e.second);}
    for (const auto& e : tl_street.list()) {res.push_back(e.styled_road.triangle_list);}
    for (const auto& e : tl_street_curb.map()) {res.push_back(e.second);}
    for (const auto& e : tl_street_curb2.map()) {res.push_back(e.second);}
    for (const auto& e : tl_air_street_curb.map()) {res.push_back(e.second);}
    return res;
}

std::list<std::shared_ptr<TriangleList<double>>> OsmTriangleLists::tls_wo_subtraction_w_water() const {
    auto res = std::list<std::shared_ptr<TriangleList<double>>>{
        tl_air_support,
        tl_tunnel_crossing,
        tl_tunnel_pipe,
        tl_racing_line};
    for (const auto& e : tl_water.map()) {res.push_back(e.second);}
    for (const auto& e : tl_terrain->map()) {if (e.first != TerrainType::BUILDING_HOLE) res.push_back(e.second);}
    for (const auto& e : tl_terrain_visuals.map()) {res.push_back(e.second);}
    for (const auto& e : tl_terrain_extrusion.map()) {res.push_back(e.second);}
    for (const auto& e : tl_street_crossing.map()) {res.push_back(e.second);}
    for (const auto& e : tl_street.list()) {res.push_back(e.styled_road.triangle_list);}
    for (const auto& e : tl_street_curb.map()) {res.push_back(e.second);}
    for (const auto& e : tl_street_curb2.map()) {res.push_back(e.second);}
    for (const auto& e : tl_air_street_curb.map()) {res.push_back(e.second);}
    return res;
}

std::list<std::shared_ptr<TriangleList<double>>> OsmTriangleLists::tls_raised() const {
    auto res = std::list<std::shared_ptr<TriangleList<double>>>{
        tl_air_support,
        tl_tunnel_crossing,
        tl_tunnel_pipe,
        tl_tunnel_bdry,
        tl_racing_line};
    for (const auto& e : tl_terrain->map()) {res.push_back(e.second);}
    for (const auto& e : tl_terrain_visuals.map()) {res.push_back(e.second);}
    for (const auto& e : tl_terrain_extrusion.map()) {res.push_back(e.second);}
    for (const auto& e : tl_street_crossing.map()) {res.push_back(e.second);}
    for (const auto& e : tl_street.list()) {res.push_back(e.styled_road.triangle_list);}
    for (const auto& e : tl_street_curb.map()) {res.push_back(e.second);}
    for (const auto& e : tl_street_curb2.map()) {res.push_back(e.second);}
    for (const auto& e : tl_air_street_curb.map()) {res.push_back(e.second);}
    return res;
}

std::list<std::shared_ptr<TriangleList<double>>> OsmTriangleLists::tls_smoothed() const {
    auto res = std::list<std::shared_ptr<TriangleList<double>>>{
        tl_air_support,
        tl_racing_line};
    for (const auto& e : tl_terrain->map()) {res.push_back(e.second);}
    for (const auto& e : tl_terrain_visuals.map()) {res.push_back(e.second);}
    for (const auto& e : tl_terrain_extrusion.map()) {res.push_back(e.second);}
    for (const auto& e : tl_street_crossing.map()) {res.push_back(e.second);}
    for (const auto& e : tl_street.list()) {res.push_back(e.styled_road.triangle_list);}
    for (const auto& e : tl_street_curb.map()) {res.push_back(e.second);}
    for (const auto& e : tl_street_curb2.map()) {res.push_back(e.second);}
    for (const auto& e : tl_air_street_curb.map()) {res.push_back(e.second);}
    return res;
}

std::list<std::shared_ptr<TriangleList<double>>> OsmTriangleLists::tls_with_vertex_normals() const {
    auto res = std::list<std::shared_ptr<TriangleList<double>>>{
        tl_tunnel_crossing};
    for (const auto& e : tl_terrain->map()) {res.push_back(e.second);}
    for (const auto& e : tl_terrain_visuals.map()) {res.push_back(e.second);}
    for (const auto& e : tl_street_crossing.map()) { if (e.first != RoadType::WALL) res.push_back(e.second);}
    for (const auto& e : tl_street.list()) { if (e.road_properties.type != RoadType::WALL) res.push_back(e.styled_road.triangle_list);}
    return res;
}

std::list<std::shared_ptr<TriangleList<double>>> OsmTriangleLists::tls_no_grass() const {
    auto res = std::list<std::shared_ptr<TriangleList<double>>>{
        tl_air_support,
        tl_tunnel_crossing,
        tl_tunnel_bdry};
    if (tl_terrain->contains(TerrainType::STONE)) res.push_back((*tl_terrain)[TerrainType::STONE]);
    if (tl_terrain->contains(TerrainType::ASPHALT)) res.push_back((*tl_terrain)[TerrainType::ASPHALT]);
    for (const auto& e : tl_street_crossing.map()) {res.push_back(e.second);}
    for (const auto& e : tl_street.list()) {res.push_back(e.styled_road.triangle_list);}
    for (const auto& e : tl_street_curb.map()) {res.push_back(e.second);}
    for (const auto& e : tl_street_curb2.map()) {res.push_back(e.second);}
    for (const auto& e : tl_air_street_curb.map()) {res.push_back(e.second);}
    return res;
}

std::list<std::shared_ptr<TriangleList<double>>> OsmTriangleLists::tls_curb_only() const {
    auto res = std::list<std::shared_ptr<TriangleList<double>>>{};
    for (const auto& e : tl_street_curb.map()) {res.push_back(e.second);}
    return res;
}

std::list<std::shared_ptr<TriangleList<double>>> OsmTriangleLists::tls_crossing_only() const {
    auto res = std::list<std::shared_ptr<TriangleList<double>>>{};
    for (const auto& e : tl_street_crossing.map()) {res.push_back(e.second);}
    return res;
}

#define INSERT(a) result.insert(result.end(), a->triangles_.begin(), a->triangles_.end())
#define INSERT2(a) for (const auto& e : a.map()) { \
    result.insert( \
        result.end(), \
        e.second->triangles_.begin(), \
        e.second->triangles_.end()); \
}
#define INSERT3(a) for (const auto& e : a.list()) { \
    result.insert( \
        result.end(), \
        e.styled_road.triangle_list->triangles_.begin(), \
        e.styled_road.triangle_list->triangles_.end()); \
}
#define INSERT4(a) for (const auto& e : a) { \
    result.insert( \
        result.end(), \
        e->triangles_.begin(), \
        e->triangles_.end()); \
}

std::list<FixedArray<ColoredVertex<double>, 3>> OsmTriangleLists::all_hole_triangles() const {
    std::list<FixedArray<ColoredVertex<double>, 3>> result;
    INSERT2(tl_street_crossing);
    INSERT3(tl_street);
    INSERT2(tl_street_curb);
    INSERT2(tl_street_curb2);
    INSERT(tl_ditch);
    INSERT(tl_entrance.at(EntranceType::TUNNEL));
    INSERT(tl_entrance.at(EntranceType::BRIDGE));
    INSERT4(tls_buildings_ground);
    return result;
}

std::list<FixedArray<ColoredVertex<double>, 3>> OsmTriangleLists::street_hole_triangles() const {
    std::list<FixedArray<ColoredVertex<double>, 3>> result;
    INSERT2(tl_street_crossing);
    INSERT3(tl_street);
    INSERT2(tl_street_curb);
    INSERT2(tl_street_curb2);
    INSERT(tl_ditch);
    INSERT(tl_entrance.at(EntranceType::TUNNEL));
    INSERT(tl_entrance.at(EntranceType::BRIDGE));
    return result;
}

std::list<FixedArray<ColoredVertex<double>, 3>> OsmTriangleLists::no_trees_triangles() const {
    std::list<FixedArray<ColoredVertex<double>, 3>> result = street_hole_triangles();
    if (tl_terrain->contains(TerrainType::FLOWERS)) INSERT((*tl_terrain)[TerrainType::FLOWERS]);
    return result;
}

std::list<FixedArray<ColoredVertex<double>, 3>> OsmTriangleLists::building_hole_triangles() const {
    std::list<FixedArray<ColoredVertex<double>, 3>> result;
    INSERT4(tls_buildings_ground);
    return result;
}

std::list<FixedArray<ColoredVertex<double>, 3>> OsmTriangleLists::street_triangles() const {
    std::list<FixedArray<ColoredVertex<double>, 3>> result;
    INSERT3(tl_street);
    return result;
}

std::list<FixedArray<ColoredVertex<double>, 3>> OsmTriangleLists::ditch_triangles() const {
    std::list<FixedArray<ColoredVertex<double>, 3>> result;
    INSERT(tl_ditch);
    return result;
}

#undef INSERT
#undef INSERT2
#undef INSERT3

namespace Mlib {

template class EntityTypeTriangleList<RoadType>;
template class EntityTypeTriangleList<TerrainType>;
template class EntityTypeTriangleList<WaterType>;

}
