#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Default_Uninitialized_Vector.hpp>
#include <Mlib/Geometry/Material/Aggregate_Mode.hpp>
#include <Mlib/Math/Interp_Fwd.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Elements.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <Mlib/Stats/Random_Number_Generators.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <list>
#include <map>
#include <set>

namespace Mlib {

template <class TDir, class TPos, size_t n>
class TransformationMatrix;
template <class TData, size_t... tshape>
class OrderableFixedArray;
class SceneNodeResources;
template <class TPos>
struct ColoredVertex;
struct Material;
struct ResourceInstanceDescriptor;
struct ObjectResourceDescriptor;
template <class TPos>
class TriangleList;
struct ParsedResourceName;
enum class DrivingDirection;
enum class WayPointLocation;
class BatchResourceInstantiator;
struct OsmTriangleLists;
struct BoundingInfo;
struct SteinerPointInfo;
class StreetBvh;
template <class TPos>
class VertexHeightBinding;
struct BarrierStyle;
struct FacadeTexture;

static const FixedArray<float, 3> way_color{1.f, 1.f, 1.f };           // replaced with texture
static const FixedArray<float, 3> racing_line_color{1.f, 1.f, 1.f };   // mixed with texture
static const FixedArray<float, 3> terrain_color{1.f, 1.f, 1.f };       // replaced with texture
static const FixedArray<float, 3> building_color{1.f, 1.f, 1.f };
static const FixedArray<float, 3> roof_color{1.f, 1.f, 1.f };
// grep highway map.osm | grep -v pedestrian | grep -v path | grep -v footway | grep -v cycleway
// static const std::set<std::string> excluded_highways = {"pedestrian", "path", "footway", "cycleway", "steps"};
// static const std::set<std::string> path_tags = {"track", "tertiary"};
static const std::set<std::string> excluded_highway_tags = {};
static const std::set<std::string> included_barriers = {"wall", "guard_rail", "fence"};
static const std::set<std::string> excluded_buildings = {"roof"};

void draw_node(
    UUVector<FixedArray<ColoredVertex<CompressedScenePos>, 3>>& triangles,
    const FixedArray<CompressedScenePos, 2>& pos2d,
    CompressedScenePos size = (CompressedScenePos)0.01f);

void draw_nodes(
    UUVector<FixedArray<ColoredVertex<CompressedScenePos>, 3>>& triangles,
    const std::map<std::string, Node>& nodes,
    const std::map<std::string, std::list<std::string>>& ways);

// void draw_test_lines(
//     TriangleList& tl,
//     float width);

std::string parse_string(
    const std::map<std::string, std::string>& tags,
    const std::string& key,
    const std::string& default_value);

template <class T>
T parse_meters(
    const std::map<std::string, std::string>& tags,
    const std::string& key,
    T default_value);

float parse_radians(
    const std::map<std::string, std::string>& tags,
    const std::string& key,
    float default_value);

FixedArray<float, 3> parse_color(
    const std::map<std::string, std::string>& tags,
    const std::string& key,
    const FixedArray<float, 3>& default_value);

float parse_float(
    const std::map<std::string, std::string>& tags,
    const std::string& key,
    float default_value);

bool parse_bool(
    const std::map<std::string, std::string>& tags,
    const std::string& key,
    bool default_value);

//class PolygonDrawer {
//public:
//    void draw_line(const p2t::Point& from, const p2t::Point& to, size_t nsteps) {
//        for (size_t i = 0; i < nsteps; ++i) {
//            double alpha = double(i) / nsteps;
//            point_list_.push_back((1 - alpha) * from + alpha * to);
//            points_.push_back(&point_list_.back());
//        }
//    }
//    std::vector<p2t::Point*> points_;
//private:
//    std::list<p2t::Point> point_list_;
//};

void raise_streets(
    const std::list<std::shared_ptr<TriangleList<CompressedScenePos>>>& tls_street_wo_curb,
    const std::list<std::shared_ptr<TriangleList<CompressedScenePos>>>& tls_ground,
    float scale,
    float amount);

void add_beacons_to_raceways(
    SceneNodeResources& scene_node_resources,
    BatchResourceInstantiator& bri,
    const std::map<std::string, Node>& nodes,
    const std::map<std::string, Way>& ways,
    float raceway_beacon_distance,
    float scale);

// void add_grass_outlines(
//     std::map<std::string, std::list<ResourceInstanceDescriptor>>& resource_instance_positions,
//     std::list<FixedArray<float, 2>>& steiner_points,
//     const std::map<std::string, Node>& nodes,
//     const std::map<std::string, Way>& ways,
//     bool continuous,
//     float tree_distance,
//     float tree_inwards_distance,
//     float scale);

// void add_binary_vegetation_old(
//     std::list<std::shared_ptr<TriangleList<double>>>& tls,
//     const Material& material,
//     const std::string& grass_texture,
//     const std::string& tree_texture,
//     const std::string& tree_texture_2,
//     const TriangleList<double>& ground_triangles,
//     float scale);

/*enum class TriangleMaterial {
    ROAD,
    TERRAIN
};

template <class TTag>
class TriangleTag {
    using O = OrderableFixedArray<float, 3>;
public:
    TTag get(const FixedArray<ColoredVertex, 3>& triangle) const {
        OrderableFixedArray<O, 3> key;
        key(0) = O{triangle(0).position};
        key(1) = O{triangle(1).position};
        key(2) = O{triangle(2).position};
        return tags_.at(key);
    }
    void set(const FixedArray<ColoredVertex, 3>& triangle, const TTag& tag) {
        OrderableFixedArray<OrderableFixedArray<float, 3>, 3> key{
            triangle(0).position,
            triangle(1).position,
            triangle(2).position};
        auto it = tags_.find(key);
        if (it != tags_.end()) {
            THROW_OR_ABORT("Tag already set");
            tags_.insert(key, tag);
        }
    }
private:
    std::map<OrderableFixedArray<OrderableFixedArray<float, 3>, 3>, TriangleMaterial> tags_;
};*/

void colorize_height_map(std::list<FixedArray<ColoredVertex<CompressedScenePos>, 3>>& triangles);

std::vector<FixedArray<CompressedScenePos, 2>> removed_duplicates(const std::vector<FixedArray<CompressedScenePos, 2>>& nodes, bool verbose = true);
std::list<FixedArray<CompressedScenePos, 2>> removed_duplicates(const std::list<FixedArray<CompressedScenePos, 2>>& nodes, bool verbose = true);
std::list<SteinerPointInfo> removed_duplicates(const std::list<SteinerPointInfo>& nodes, bool verbose = true);

void check_curb_validity(float curb_alpha, float curb2_alpha);

}
