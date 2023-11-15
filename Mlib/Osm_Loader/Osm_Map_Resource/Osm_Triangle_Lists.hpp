#pragma once
#include <list>
#include <map>
#include <memory>
#include <set>
#include <string>

namespace Mlib {

template <class TPos>
class TriangleList;
struct OsmResourceConfig;
struct Material;
template <class TData, size_t... tshape>
class OrderableFixedArray;
enum class EntranceType;
struct RoadProperties;
enum class RoadType;
template <class TPos>
struct ColoredVertex;
template <typename TData, size_t... tshape>
class FixedArray;
struct StyledRoad;
struct StyledRoadEntry;
enum class TerrainType;
enum class WaterType;

class RoadPropertiesTriangleList {
public:
    void append(const StyledRoadEntry& entry);
    const StyledRoad& operator [] (const RoadProperties& road_properties) const;
    const std::list<StyledRoadEntry>& list() const;
private:
    std::list<StyledRoadEntry> lst_;
};

template <class EntityType>
class EntityTypeTriangleList {
public:
    void insert(EntityType road_type, const std::shared_ptr<TriangleList<double>>& lst);
    bool contains(EntityType road_type) const;
    const std::shared_ptr<TriangleList<double>>& operator [] (EntityType road_type) const;
    const std::map<EntityType, std::shared_ptr<TriangleList<double>>>& map() const;
    template <class Archive>
    void serialize(Archive& archive) {
        archive(lst_);
    }
private:
    std::map<EntityType, std::shared_ptr<TriangleList<double>>> lst_;
};

typedef EntityTypeTriangleList<RoadType> RoadTypeTriangleList;
typedef EntityTypeTriangleList<TerrainType> TerrainTypeTriangleList;
typedef EntityTypeTriangleList<WaterType> WaterTypeTriangleList;

struct OsmTriangleLists {
    OsmTriangleLists(
        const OsmResourceConfig& config,
        const std::string& name_suffix);
    ~OsmTriangleLists();
    std::shared_ptr<TerrainTypeTriangleList> tl_terrain;
    TerrainTypeTriangleList tl_terrain_visuals;
    TerrainTypeTriangleList tl_terrain_extrusion;
    RoadPropertiesTriangleList tl_street;
    RoadTypeTriangleList tl_street_visuals;
    RoadTypeTriangleList tl_street_crossing;
    RoadTypeTriangleList tl_street_curb;
    RoadTypeTriangleList tl_street_curb2;
    RoadTypeTriangleList tl_air_street_curb;
    std::shared_ptr<TriangleList<double>> tl_racing_line;
    std::shared_ptr<TriangleList<double>> tl_air_support;
    std::shared_ptr<TriangleList<double>> tl_tunnel_pipe;
    std::shared_ptr<TriangleList<double>> tl_tunnel_bdry;
    std::shared_ptr<TriangleList<double>> tl_tunnel_crossing;
    std::shared_ptr<TriangleList<double>> tl_ditch;
    std::map<EntranceType, std::shared_ptr<TriangleList<double>>> tl_entrance;
    std::map<EntranceType, std::set<OrderableFixedArray<double, 2>>> entrances;
    WaterTypeTriangleList tl_water;
    std::list<std::shared_ptr<TriangleList<double>>> tls_buildings_ground;
    std::list<std::shared_ptr<TriangleList<double>>> tls_ocean_ground;
    void insert(const OsmTriangleLists& other);
    std::list<std::shared_ptr<TriangleList<double>>> tls_street_wo_curb() const;
    std::list<std::shared_ptr<TriangleList<double>>> tls_street_wo_curb_follower() const;
    std::list<std::shared_ptr<TriangleList<double>>> tls_wall_wo_curb() const;
    std::list<std::shared_ptr<TriangleList<double>>> tls_street() const;
    std::list<std::shared_ptr<TriangleList<double>>> tls_terrain_nosmooth() const;
    std::list<std::shared_ptr<TriangleList<double>>> tls_wo_subtraction_and_water() const;
    std::list<std::shared_ptr<TriangleList<double>>> tls_wo_subtraction_w_water() const;
    std::list<std::shared_ptr<TriangleList<double>>> tls_raised() const;
    std::list<std::shared_ptr<TriangleList<double>>> tls_smoothed() const;
    std::list<std::shared_ptr<TriangleList<double>>> tls_smooth() const;
    std::list<std::shared_ptr<TriangleList<double>>> tls_no_backfaces() const;
    std::list<std::shared_ptr<TriangleList<double>>> tls_with_vertex_normals() const;
    std::list<std::shared_ptr<TriangleList<double>>> tls_no_grass() const;
    std::list<std::shared_ptr<TriangleList<double>>> tls_curb_only() const;
    std::list<std::shared_ptr<TriangleList<double>>> tls_crossing_only() const;
    std::list<FixedArray<ColoredVertex<double>, 3>> all_hole_triangles() const;
    std::list<FixedArray<ColoredVertex<double>, 3>> street_hole_triangles() const;
    std::list<FixedArray<ColoredVertex<double>, 3>> no_trees_triangles() const;
    std::list<FixedArray<ColoredVertex<double>, 3>> building_hole_triangles() const;
    std::list<FixedArray<ColoredVertex<double>, 3>> ocean_ground_triangles() const;
    std::list<FixedArray<ColoredVertex<double>, 3>> street_triangles() const;
    std::list<FixedArray<ColoredVertex<double>, 3>> street_triangles(RoadType road_type) const;
    std::list<FixedArray<ColoredVertex<double>, 3>> ditch_triangles() const;
    bool has_curb() const;
};

}
