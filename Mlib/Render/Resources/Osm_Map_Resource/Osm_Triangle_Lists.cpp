#include "Osm_Triangle_Lists.hpp"
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Entrance_Type.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Osm_Resource_Config.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Road_Type.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Styled_Road.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Terrain_Type.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Water_Type.hpp>

using namespace Mlib;

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
void EntityTypeTriangleList<EntityType>::insert(EntityType road_type, const std::shared_ptr<TriangleList>& lst) {
    if (!lst_.insert({road_type, lst}).second) {
        throw std::runtime_error("Could not insert triangle list");
    }
}

template <class EntityType>
bool EntityTypeTriangleList<EntityType>::contains(EntityType road_type) const {
    return lst_.find(road_type) != lst_.end();
}

template <class EntityType>
const std::shared_ptr<TriangleList>& EntityTypeTriangleList<EntityType>::operator [] (EntityType road_type) const {
    auto it = lst_.find(road_type);
    if (it == lst_.end()) {
        throw std::runtime_error("Could not find list with type " + to_string(road_type));
    }
    return it->second;
}

template <class EntityType>
const std::map<EntityType, std::shared_ptr<TriangleList>>& EntityTypeTriangleList<EntityType>::map() const {
    return lst_;
}

OsmTriangleLists::OsmTriangleLists(const OsmResourceConfig& config)
{
    auto primary_rendering_resources = RenderingContextStack::primary_rendering_resources();
    tl_terrain = std::make_shared<TerrainTypeTriangleList>();
    tl_terrain->insert(TerrainType::HOLE, std::make_shared<TriangleList>(terrain_type_to_string(TerrainType::HOLE), Material()));
    for (auto& ttt : config.terrain_textures) {
        tl_terrain->insert(ttt.first, std::make_shared<TriangleList>(terrain_type_to_string(ttt.first), Material{
            .dirt_texture = config.dirt_texture,
            .occluded_type = OccludedType::LIGHT_MAP_COLOR,
            .occluder_type = OccluderType::WHITE,
            .aggregate_mode = config.blend_street ? AggregateMode::ONCE : AggregateMode::OFF,
            .specularity = {0.f, 0.f, 0.f},
            .draw_distance_noperations = 1000}.compute_color_mode()));
        tl_terrain_visuals.insert(ttt.first, std::make_shared<TriangleList>(terrain_type_to_string(ttt.first) + "_visuals", Material{
            .dirt_texture = config.dirt_texture,
            .occluded_type = OccludedType::LIGHT_MAP_COLOR,
            .occluder_type = OccluderType::WHITE,
            .collide = false,
            .aggregate_mode = config.blend_street ? AggregateMode::ONCE : AggregateMode::OFF,
            .specularity = {0.f, 0.f, 0.f},
            .draw_distance_noperations = 1000}.compute_color_mode()));
        tl_terrain_extrusion.insert(ttt.first, std::make_shared<TriangleList>(terrain_type_to_string(ttt.first) + "_street_extrusion", Material{
            .dirt_texture = config.dirt_texture,
            .occluded_type = OccludedType::LIGHT_MAP_COLOR,
            .occluder_type = OccluderType::WHITE,
            .aggregate_mode = config.blend_street ? AggregateMode::ONCE : AggregateMode::OFF,
            .specularity = {0.f, 0.f, 0.f},
            .draw_distance_noperations = 1000}.compute_color_mode()));
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
        tl_street_crossing.insert(s.first, std::make_shared<TriangleList>(road_type_to_string(s.first), Material{
            .textures = {primary_rendering_resources->get_blend_map_texture(s.second)},
            .occluded_type = (s.first != RoadType::WALL) ? OccludedType::LIGHT_MAP_COLOR : OccludedType::OFF,
            .occluder_type = (s.first != RoadType::WALL) ? OccluderType::WHITE : OccluderType::BLACK,
            .specularity = OrderableFixedArray<float, 3>{fixed_full<float, 3>((float)(s.first != RoadType::WALL))},
            .draw_distance_noperations = 1000}.compute_color_mode()));
    }
    for (const auto& s : config.street_texture) {
        tl_street.append(StyledRoadEntry{
            .road_properties = s.first,
            .styled_road = StyledRoad{
                .triangle_list = std::make_shared<TriangleList>((std::string)s.first, Material{
                    .continuous_blending_z_order = config.blend_street ? 1 : 0,
                    .blend_mode = config.blend_street ? BlendMode::CONTINUOUS : BlendMode::OFF,
                    .textures = {primary_rendering_resources->get_blend_map_texture(s.second.texture)},
                    .occluded_type = (s.first.type != RoadType::WALL) ? OccludedType::LIGHT_MAP_COLOR : OccludedType::OFF,
                    .occluder_type = (s.first.type != RoadType::WALL) ? OccluderType::WHITE : OccluderType::BLACK,
                    .depth_func_equal = config.blend_street,
                    .aggregate_mode = config.blend_street ? AggregateMode::ONCE : AggregateMode::OFF,
                    .specularity = OrderableFixedArray<float, 3>{fixed_full<float, 3>((float)(s.first.type != RoadType::WALL))},
                    .draw_distance_noperations = 1000}.compute_color_mode()),
                .uvx = s.second.uvx}}); // mixed_texture: terrain_texture
    }
    WrapMode curb_wrap_mode_s = (config.extrude_curb_amount != 0) || ((config.curb_alpha != 1) && (config.extrude_street_amount != 0)) ? WrapMode::REPEAT : WrapMode::CLAMP_TO_EDGE;
    for (const auto& s : config.curb_street_texture) {
        tl_street_curb.insert(s.first, std::make_shared<TriangleList>("curb_" + road_type_to_string(s.first), Material{
            .textures = {primary_rendering_resources->get_blend_map_texture(s.second)},
            .occluded_type = (s.first != RoadType::WALL) ? OccludedType::LIGHT_MAP_COLOR : OccludedType::OFF,
            .occluder_type = (s.first != RoadType::WALL) ? OccluderType::WHITE : OccluderType::BLACK,
            .wrap_mode_s = curb_wrap_mode_s,
            .specularity = OrderableFixedArray{fixed_full<float, 3>((float)(config.extrude_curb_amount == 0 && s.first != RoadType::WALL))},
            .draw_distance_noperations = 1000}.compute_color_mode())); // mixed_texture: terrain_texture
    }
    for (const auto& s : config.curb2_street_texture) {
        tl_street_curb2.insert(s.first, std::make_shared<TriangleList>("curb2_" + road_type_to_string(s.first), Material{
            .textures = {primary_rendering_resources->get_blend_map_texture(s.second)},
            .occluded_type = (s.first != RoadType::WALL) ? OccludedType::LIGHT_MAP_COLOR : OccludedType::OFF,
            .occluder_type = (s.first != RoadType::WALL) ? OccluderType::WHITE : OccluderType::BLACK,
            .specularity = OrderableFixedArray<float, 3>{fixed_full<float, 3>((float)(s.first != RoadType::WALL))},
            .draw_distance_noperations = 1000}.compute_color_mode())); // mixed_texture: terrain_texture
    }
    for (const auto& s : config.air_curb_street_texture) {
        tl_air_street_curb.insert(s.first, std::make_shared<TriangleList>("air_curb_" + road_type_to_string(s.first), Material{
            .textures = {primary_rendering_resources->get_blend_map_texture(s.second)},
            .occluded_type = OccludedType::LIGHT_MAP_COLOR,
            .occluder_type = OccluderType::WHITE,
            .wrap_mode_s = curb_wrap_mode_s,
            .draw_distance_noperations = 1000}.compute_color_mode()));
    }
    tl_air_support = std::make_shared<TriangleList>("air_support", Material{
        .textures = {primary_rendering_resources->get_blend_map_texture(config.air_support_texture)},
        .occluded_type = OccludedType::LIGHT_MAP_COLOR,
        .occluder_type = OccluderType::WHITE,
        .draw_distance_noperations = 1000}.compute_color_mode());
    tl_tunnel_crossing = std::make_shared<TriangleList>("tunnel_crossing", Material{
        .textures = {primary_rendering_resources->get_blend_map_texture(config.tunnel_pipe_texture)},
        .occluded_type = OccludedType::LIGHT_MAP_COLOR,
        .occluder_type = OccluderType::WHITE,
        .specularity = {0.f, 0.f, 0.f},
        .draw_distance_noperations = 1000}.compute_color_mode());
    tl_tunnel_pipe = std::make_shared<TriangleList>("tunnel_pipe", Material{
        .textures = {primary_rendering_resources->get_blend_map_texture(config.tunnel_pipe_texture)},
        .occluded_type = OccludedType::LIGHT_MAP_COLOR,
        .occluder_type = OccluderType::WHITE,
        .specularity = {0.f, 0.f, 0.f},
        .draw_distance_noperations = 1000}.compute_color_mode());
    tl_tunnel_bdry = std::make_shared<TriangleList>("tunnel_bdry", Material());
    tl_entrance[EntranceType::TUNNEL] = std::make_shared<TriangleList>("tunnel_entrance", Material());
    tl_entrance[EntranceType::BRIDGE] = std::make_shared<TriangleList>("bridge_entrance", Material());
    entrances[EntranceType::TUNNEL];
    entrances[EntranceType::BRIDGE];
    tl_water.insert(WaterType::UNDEFINED, std::make_shared<TriangleList>("water", Material{
        .textures = {primary_rendering_resources->get_blend_map_texture(config.water_texture)},
        .collide = false,
        .draw_distance_noperations = 1000}.compute_color_mode()));
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

std::list<std::shared_ptr<TriangleList>> OsmTriangleLists::tls_street_wo_curb() const {
    auto res = std::list<std::shared_ptr<TriangleList>>{};
    for (const auto& e : tl_street_crossing.map()) {if (e.first != RoadType::WALL) res.push_back(e.second);}
    for (const auto& e : tl_street.list()) {if (e.road_properties.type != RoadType::WALL) res.push_back(e.styled_road.triangle_list);}
    return res;
}

std::list<std::shared_ptr<TriangleList>> OsmTriangleLists::tls_wall_wo_curb() const {
    auto res = std::list<std::shared_ptr<TriangleList>>{};
    for (const auto& e : tl_street_crossing.map()) {if (e.first == RoadType::WALL) res.push_back(e.second);}
    for (const auto& e : tl_street.list()) {if (e.road_properties.type == RoadType::WALL) res.push_back(e.styled_road.triangle_list);}
    return res;
}

std::list<std::shared_ptr<TriangleList>> OsmTriangleLists::tls_street() const {
    auto res = std::list<std::shared_ptr<TriangleList>>{
        tl_air_support};
    for (const auto& e : tl_street_crossing.map()) {res.push_back(e.second);}
    for (const auto& e : tl_street.list()) {res.push_back(e.styled_road.triangle_list);}
    for (const auto& e : tl_street_curb.map()) {res.push_back(e.second);}
    for (const auto& e : tl_street_curb2.map()) {res.push_back(e.second);}
    for (const auto& e : tl_air_street_curb.map()) {res.push_back(e.second);}
    return res;
}

std::list<std::shared_ptr<TriangleList>> OsmTriangleLists::tls_smooth() const {
    std::list<std::shared_ptr<TriangleList>> res;
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

std::list<std::shared_ptr<TriangleList>> OsmTriangleLists::tls_no_backfaces() const {
    auto res = std::list<std::shared_ptr<TriangleList>>{
        tl_air_support,
        tl_tunnel_crossing};
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

std::list<std::shared_ptr<TriangleList>> OsmTriangleLists::tls_wo_subtraction_and_water() const {
    auto res = std::list<std::shared_ptr<TriangleList>>{
        tl_air_support,
        tl_tunnel_crossing,
        tl_tunnel_pipe};
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

std::list<std::shared_ptr<TriangleList>> OsmTriangleLists::tls_wo_subtraction_w_water() const {
    auto res = std::list<std::shared_ptr<TriangleList>>{
        tl_air_support,
        tl_tunnel_crossing,
        tl_tunnel_pipe};
    for (const auto& e : tl_water.map()) {res.push_back(e.second);}
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

std::list<std::shared_ptr<TriangleList>> OsmTriangleLists::tls_raised() const {
    auto res = std::list<std::shared_ptr<TriangleList>>{
        tl_air_support,
        tl_tunnel_crossing,
        tl_tunnel_pipe,
        tl_tunnel_bdry};
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

std::list<std::shared_ptr<TriangleList>> OsmTriangleLists::tls_smoothed() const {
    auto res = std::list<std::shared_ptr<TriangleList>>{
        tl_air_support};
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

std::list<std::shared_ptr<TriangleList>> OsmTriangleLists::tls_with_vertex_normals() const {
    auto res = std::list<std::shared_ptr<TriangleList>>{
        tl_tunnel_crossing};
    for (const auto& e : tl_terrain->map()) {res.push_back(e.second);}
    for (const auto& e : tl_terrain_visuals.map()) {res.push_back(e.second);}
    for (const auto& e : tl_street_crossing.map()) { if (e.first != RoadType::WALL) res.push_back(e.second);}
    for (const auto& e : tl_street.list()) { if (e.road_properties.type != RoadType::WALL) res.push_back(e.styled_road.triangle_list);}
    return res;
}

std::list<std::shared_ptr<TriangleList>> OsmTriangleLists::tls_no_grass() const {
    auto res = std::list<std::shared_ptr<TriangleList>>{
        tl_air_support,
        tl_tunnel_crossing,
        tl_tunnel_bdry};
    for (const auto& e : tl_street_crossing.map()) {res.push_back(e.second);}
    for (const auto& e : tl_street.list()) {res.push_back(e.styled_road.triangle_list);}
    for (const auto& e : tl_street_curb.map()) {res.push_back(e.second);}
    for (const auto& e : tl_street_curb2.map()) {res.push_back(e.second);}
    for (const auto& e : tl_air_street_curb.map()) {res.push_back(e.second);}
    return res;
}

std::list<std::shared_ptr<TriangleList>> OsmTriangleLists::tls_curb_only() const {
    auto res = std::list<std::shared_ptr<TriangleList>>{};
    for (const auto& e : tl_street_curb.map()) {res.push_back(e.second);}
    return res;
}

std::list<std::shared_ptr<TriangleList>> OsmTriangleLists::tls_crossing_only() const {
    auto res = std::list<std::shared_ptr<TriangleList>>{};
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

std::list<FixedArray<ColoredVertex, 3>> OsmTriangleLists::hole_triangles() const {
    std::list<FixedArray<ColoredVertex, 3>> result;
    INSERT2(tl_street_crossing);
    INSERT3(tl_street);
    INSERT2(tl_street_curb);
    INSERT2(tl_street_curb2);
    INSERT(tl_entrance.at(EntranceType::TUNNEL));
    INSERT(tl_entrance.at(EntranceType::BRIDGE));
    INSERT4(tls_buildings_ground);
    return result;
}

std::list<FixedArray<ColoredVertex, 3>> OsmTriangleLists::street_triangles() const {
    std::list<FixedArray<ColoredVertex, 3>> result;
    INSERT3(tl_street);
    return result;
}

#undef INSERT
#undef INSERT2
#undef INSERT3

template class EntityTypeTriangleList<RoadType>;
template class EntityTypeTriangleList<TerrainType>;
template class EntityTypeTriangleList<WaterType>;
