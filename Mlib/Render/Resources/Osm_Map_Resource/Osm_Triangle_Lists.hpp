#pragma once
#include <list>
#include <map>
#include <memory>
#include <set>

namespace Mlib {

class TriangleList;
struct OsmResourceConfig;
struct Material;
template <class TData, size_t... tshape>
class OrderableFixedArray;
enum class EntranceType;
struct RoadProperties;
enum class RoadType;
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
    void insert(EntityType road_type, const std::shared_ptr<TriangleList>& lst);
    bool contains(EntityType road_type) const;
    const std::shared_ptr<TriangleList>& operator [] (EntityType road_type) const;
    const std::map<EntityType, std::shared_ptr<TriangleList>>& map() const;
private:
    std::map<EntityType, std::shared_ptr<TriangleList>> lst_;
};

typedef EntityTypeTriangleList<RoadType> RoadTypeTriangleList;
typedef EntityTypeTriangleList<TerrainType> TerrainTypeTriangleList;
typedef EntityTypeTriangleList<WaterType> WaterTypeTriangleList;

struct OsmTriangleLists {
    explicit OsmTriangleLists(const OsmResourceConfig& config);
    ~OsmTriangleLists();
    std::shared_ptr<TerrainTypeTriangleList> tl_terrain;
    TerrainTypeTriangleList tl_terrain_visuals;
    TerrainTypeTriangleList tl_terrain_extrusion;
    RoadPropertiesTriangleList tl_street;
    RoadTypeTriangleList tl_street_crossing;
    RoadTypeTriangleList tl_street_curb;
    RoadTypeTriangleList tl_street_curb2;
    RoadTypeTriangleList tl_air_street_curb;
    std::shared_ptr<TriangleList> tl_air_support;
    std::shared_ptr<TriangleList> tl_tunnel_pipe;
    std::shared_ptr<TriangleList> tl_tunnel_bdry;
    std::shared_ptr<TriangleList> tl_tunnel_crossing;
    std::map<EntranceType, std::shared_ptr<TriangleList>> tl_entrance;
    std::map<EntranceType, std::set<OrderableFixedArray<float, 2>>> entrances;
    WaterTypeTriangleList tl_water;
    std::list<std::shared_ptr<TriangleList>> tls_buildings_ground;
    void insert(const OsmTriangleLists& other);
    std::list<std::shared_ptr<TriangleList>> tls_street_wo_curb() const;
    std::list<std::shared_ptr<TriangleList>> tls_wall_wo_curb() const;
    std::list<std::shared_ptr<TriangleList>> tls_street() const;
    std::list<std::shared_ptr<TriangleList>> tls_wo_subtraction_and_water() const;
    std::list<std::shared_ptr<TriangleList>> tls_wo_subtraction_w_water() const;
    std::list<std::shared_ptr<TriangleList>> tls_raised() const;
    std::list<std::shared_ptr<TriangleList>> tls_smoothed() const;
    std::list<std::shared_ptr<TriangleList>> tls_smooth() const;
    std::list<std::shared_ptr<TriangleList>> tls_no_backfaces() const;
    std::list<std::shared_ptr<TriangleList>> tls_with_vertex_normals() const;
    std::list<std::shared_ptr<TriangleList>> tls_no_grass() const;
    std::list<std::shared_ptr<TriangleList>> tls_curb_only() const;
    std::list<std::shared_ptr<TriangleList>> tls_crossing_only() const;
    std::list<FixedArray<ColoredVertex, 3>> hole_triangles() const;
    std::list<FixedArray<ColoredVertex, 3>> street_triangles() const;
};

}
