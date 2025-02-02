#include "Osm_Map_Resource_Helpers.hpp"
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Geometry/Static_Face_Lighting.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Steiner_Point_Info.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Subdivided_Way.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Scene_Graph/Resources/Batch_Resource_Instantiator.hpp>
#include <Mlib/Scene_Graph/Resources/Parsed_Resource_Name.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Strings/String.hpp>
#include <Mlib/Strings/To_Number.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <Mlib/Try_Find.hpp>

using namespace Mlib;

// std::map<OrderableFixedArray<float, 3>, SteinerPointInfo*> Mlib::gen_steiner_point_map(std::list<SteinerPointInfo>& steiner_points) {
//     std::map<OrderableFixedArray<float, 3>, SteinerPointInfo*> steiner_point_map;
//     for (auto& p : steiner_points) {
//         if (!steiner_point_map.insert({OrderableFixedArray{p.position}, &p}).second) {
//             THROW_OR_ABORT("Could not generate steiner point map");
//         }
//     }
//     return steiner_point_map;
// }
// 
// std::map<OrderableFixedArray<float, 3>, const SteinerPointInfo*> Mlib::gen_const_steiner_point_map(const std::list<SteinerPointInfo>& steiner_points) {
//     std::map<OrderableFixedArray<float, 3>, const SteinerPointInfo*> steiner_point_map;
//     for (auto& p : steiner_points) {
//         if (!steiner_point_map.insert({OrderableFixedArray{p.position}, &p}).second) {
//             THROW_OR_ABORT("Could not generate const steiner point map");
//         }
//     }
//     return steiner_point_map;
// }

void Mlib::draw_node(
    UUVector<FixedArray<ColoredVertex<CompressedScenePos>, 3>>& triangles,
    const FixedArray<CompressedScenePos, 2>& pos2d,
    CompressedScenePos size)
{
    ColoredVertex<CompressedScenePos> v00{
        FixedArray<CompressedScenePos, 3>{pos2d(0) - size, pos2d(1) - size, (CompressedScenePos)0.f},
        Colors::YELLOW,
        fixed_zeros<float, 2>(),
        fixed_zeros<float, 3>(),
        fixed_zeros<float, 3>()};
    ColoredVertex<CompressedScenePos> v01{
        FixedArray<CompressedScenePos, 3>{pos2d(0) - size, pos2d(1) + size, (CompressedScenePos)0.f},
        Colors::PURPLE,
        fixed_zeros<float, 2>(),
        fixed_zeros<float, 3>(),
        fixed_zeros<float, 3>()};
    ColoredVertex<CompressedScenePos> v10{
        FixedArray<CompressedScenePos, 3>{pos2d(0) + size, pos2d(1) - size, (CompressedScenePos)0.f},
        Colors::CYAN,
        fixed_zeros<float, 2>(),
        fixed_zeros<float, 3>(),
        fixed_zeros<float, 3>()};
    ColoredVertex<CompressedScenePos> v11{
        FixedArray<CompressedScenePos, 3>{pos2d(0) + size, pos2d(1) + size, (CompressedScenePos)0.f},
        Colors::WHITE,
        fixed_zeros<float, 2>(),
        fixed_zeros<float, 3>(),
        fixed_zeros<float, 3>()};

    triangles.push_back(FixedArray<ColoredVertex<CompressedScenePos>, 3>{v00, v11, v01});
    triangles.push_back(FixedArray<ColoredVertex<CompressedScenePos>, 3>{v11, v00, v10});
}

void Mlib::draw_nodes(
    UUVector<FixedArray<ColoredVertex<CompressedScenePos>, 3>>& triangles,
    const std::map<std::string, Node>& nodes,
    const std::map<std::string, std::list<std::string>>& ways)
{
    for (const auto& way : ways) {
        for (const auto& nd : way.second) {
            if (nodes.find(nd) == nodes.end()) {
                THROW_OR_ABORT("Way " + way.first + " could not find node with ID " + nd);
            }
            FixedArray<CompressedScenePos, 2> pos2d = nodes.at(nd).position;
            draw_node(triangles, pos2d);
        }
    }
}

// void Mlib::draw_test_lines(
//     TriangleList& tl,
//     float width)
// {
//     Rectangle rect;
//     if (!Rectangle::from_line(
//         rect,
//         FixedArray<float, 2>{0, 0},
//         FixedArray<float, 2>{0, 0},
//         FixedArray<float, 2>{0.1, 0},
//         FixedArray<float, 2>{0.2, 0},
//         FixedArray<float, 2>{0.3, 0},
//         FixedArray<float, 2>{0.3, 0},
//         width,
//         width,
//         width,
//         width,
//         width))
//     {
//         THROW_OR_ABORT("from_line failed");
//     }
//     rect.draw_z0(tl);
//     if (!Rectangle::from_line(
//         rect,
//         FixedArray<float, 2>{-0.1, 0},
//         FixedArray<float, 2>{-0.1, 0},
//         FixedArray<float, 2>{-0.2, 0},
//         FixedArray<float, 2>{-0.3, 0},
//         FixedArray<float, 2>{-0.4, 0},
//         FixedArray<float, 2>{-0.4, 0},
//         width,
//         width,
//         width,
//         width,
//         width));
//     {
//         THROW_OR_ABORT("from_line failed");
//     }
//     rect.draw_z0(tl);
// }

std::string Mlib::parse_string(
    const std::map<std::string, std::string>& tags,
    const std::string& key,
    const std::string& default_value)
{
    auto* value = try_find(tags, key);
    if (value == nullptr) {
        return default_value;
    }
    return *value;
}

template <class T>
T Mlib::parse_meters(
    const std::map<std::string, std::string>& tags,
    const std::string& key,
    T default_value)
{
    auto* value = try_find(tags, key);
    if (value == nullptr) {
        return default_value;
    }
    static const DECLARE_REGEX(re, "^([\\d.-]+) *(m|'|ft)?");
    Mlib::re::smatch match;
    if (!Mlib::re::regex_match(*value, match, re)) {
        THROW_OR_ABORT("Could not parse \"" + key + "\" value: \"" + *value + '"');
    }
    T res = safe_sto<T>(match[1].str());
    if ((match[2].str() == "'") ||
        (match[2].str() == "ft"))
    {
        res *= 0.3048f;
    }
    return res;
}

float Mlib::parse_radians(
    const std::map<std::string, std::string>& tags,
    const std::string& key,
    float default_value)
{
    auto* value = try_find(tags, key);
    if (value == nullptr) {
        return default_value;
    }
    static const DECLARE_REGEX(re, "^([\\d.-]+) *(?:°)?");
    Mlib::re::smatch match;
    if (!Mlib::re::regex_match(*value, match, re)) {
        THROW_OR_ABORT("Could not parse \"" + key + "\" value: \"" + *value + '"');
    }
    return safe_stof(match[1].str()) * float(M_PI / 180.);
}

FixedArray<float, 3> Mlib::parse_color(
    const std::map<std::string, std::string>& tags,
    const std::string& key,
    const FixedArray<float, 3>& default_value)
{
    auto rgb = try_find(tags, key);
    if (rgb == nullptr) {
        return default_value;
    }
    auto l = string_to_vector(*rgb, safe_stof);
    if (l.size() != 3) {
        THROW_OR_ABORT("\"color\" tag does not have 3 values");
    }
    return { l[0], l[1], l[2] };
}

float Mlib::parse_float(
    const std::map<std::string, std::string>& tags,
    const std::string& key,
    float default_value)
{
    auto* value = try_find(tags, key);
    if (value == nullptr) {
        return default_value;
    }
    return safe_stof(*value);
}

bool Mlib::parse_bool(
    const std::map<std::string, std::string>& tags,
    const std::string& key,
    bool default_value)
{
    auto* value = try_find(tags, key);
    if (value == nullptr) {
        return default_value;
    }
    return safe_stob(*value);
}

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

void Mlib::raise_streets(
    const std::list<std::shared_ptr<TriangleList<CompressedScenePos>>>& tls_street_wo_curb,
    const std::list<std::shared_ptr<TriangleList<CompressedScenePos>>>& tls_ground,
    float scale,
    float amount)
{
    std::set<OrderableFixedArray<CompressedScenePos, 3>> raised_nodes;
    for (auto& l : tls_street_wo_curb) {
        for (const auto& n : l->triangles) {
            raised_nodes.insert(OrderableFixedArray{n(0).position});
            raised_nodes.insert(OrderableFixedArray{n(1).position});
            raised_nodes.insert(OrderableFixedArray{n(2).position});
        }
    }
    for (auto& l : tls_ground) {
        for (auto& n : l->triangles) {
            for (auto& v : n.flat_iterable()) {
                if (raised_nodes.find(OrderableFixedArray{v.position}) != raised_nodes.end()) {
                    v.position(2) += (CompressedScenePos)(scale * amount);
                }
            }
        }
    }
}

void Mlib::add_beacons_to_raceways(
    SceneNodeResources& scene_node_resources,
    BatchResourceInstantiator& bri,
    const std::map<std::string, Node>& nodes,
    const std::map<std::string, Way>& ways,
    float raceway_beacon_distance,
    float scale)
{
    auto resource_name = VariableAndHash<std::string>{ "raceway_beacon" };
    ParsedResourceName prn{
        .name = resource_name,
        .billboard_id = BILLBOARD_ID_NONE,
        .yangle = 0.f,
        .probability = NAN,
        .aggregate_mode = scene_node_resources.aggregate_mode(*resource_name),
        .create_imposter = false,
        .max_imposter_texture_size = 0,
        .hitbox = "",
        .supplies_cooldown = NAN};
    for (const auto& w : ways) {
        const auto& tags = w.second.tags;
        if (tags.contains("raceway", "yes"))
        {
            auto sw = subdivided_way(nodes, w.second.nd, scale, raceway_beacon_distance);
            for (const auto& p : sw) {
                bri.add_parsed_resource_name(p, (CompressedScenePos)0.f, prn, 0.f, 1.f);
            }
        }
    }
}

// void Mlib::add_grass_outlines(
//     std::map<std::string, std::list<ResourceInstanceDescriptor>>& resource_instance_positions,
//     std::list<FixedArray<float, 2>>& steiner_points,
//     const std::map<std::string, Node>& nodes,
//     const std::map<std::string, Way>& ways,
//     bool continuous,
//     float tree_distance,
//     float tree_inwards_distance,
//     float scale)
// {
//     FastNormalRandomNumberGenerator<float> rng{0, 4.9, 0.2};
//     FastNormalRandomNumberGenerator<float> rng2{0, 0, 1.2};
//     GammaRandomNumberGenerator<float> rng3{0, 3, 2};
//     NextGrassResourceName ntrn{continuous};
//     for (float f : Linspace<float>(tree_inwards_distance, tree_inwards_distance + 5.f, 3)) {
//         for (const auto& w : ways) {
//             const auto& tags = w.second.tags;
//             if ((tags.find("landuse") != tags.end() && (tags.at("landuse") == "farmland" || tags.at("landuse") == "meadow")))
//             {
//                 float area = compute_area_clockwise(w.second.nd, nodes, scale);
//                 for (auto it = w.second.nd.begin(); it != w.second.nd.end(); ++it) {
//                     auto s = it;
//                     ++s;
//                     if (s == w.second.nd.end()) {
//                         continue;
//                     }
//                     FixedArray<float, 2> p0 = nodes.at(*it).position;
//                     FixedArray<float, 2> p1 = nodes.at(*s).position;
//                     float len = std::sqrt(sum(squared(p0 - p1)));
//                     FixedArray<float, 2> n{p0(1) - p1(1), p1(0) - p0(0)};
//                     n /= len;
//                     for (float a = 0.1; a < 0.91; a += tree_distance * scale / len) {
//                         float aa = a + rng2() * tree_distance * scale / len;
//                         if (aa < 0 || aa > 0.91) {
//                             continue;
//                         }
//                         FixedArray<float, 2> p = (aa * p0 + (1 - aa) * p1) + (f + rng3()) * scale * n * sign(area);
//                         resource_instance_positions.push_back({FixedArray<float, 3>{p(0), p(1), 0}, ntrn(), rng()});
//                     }
//                 }
//             }
//         }
//     }
// }

// void Mlib::add_binary_vegetation_old(
//     std::list<std::shared_ptr<TriangleList<double>>>& tls,
//     const Material& material,
//     const std::string& grass_texture,
//     const std::string& tree_texture,
//     const std::string& tree_texture_2,
//     const TriangleList<double>& ground_triangles,
//     float scale)
// {
//     size_t tid = 0;
//     for (auto& t : ground_triangles.triangles_) {
//         ++tid;
//         tls.push_back(std::make_shared<TriangleList<double>>(
//             "binary_vegetation_old",
//             material,
//             PhysicsMaterial::ATTR_VISIBLE));
//         float veg_size;
//         switch (tid % 10) {
//             case 0:
//                 tls.back()->material_.textures = { {.texture_descriptor = {.color = grass_texture}} };
//                 veg_size = 1;
//                 break;
//             case 2:
//                 tls.back()->material_.textures = { {.texture_descriptor = {.color = tree_texture}} };
//                 veg_size = 5;
//                 break;
//             case 4:
//                 tls.back()->material_.textures = { {.texture_descriptor = {.color = tree_texture_2 }} };
//                 veg_size = 5;
//                 break;
//             default:
//                 continue;
//                 // THROW_OR_ABORT("Internal error");
//         }
//         auto center = (t(0).position + t(1).position + t(2).position) / 3.f;
//         tls.back()->draw_rectangle_wo_normals(
//             center + FixedArray<float, 3>{-scale * veg_size, 0.f, 0.f},
//             center + FixedArray<float, 3>{scale * veg_size, 0.f, 0.f},
//             center + FixedArray<float, 3>{scale * veg_size, 0.f, 2 * scale * veg_size},
//             center + FixedArray<float, 3>{-scale * veg_size, 0.f, 2 * scale * veg_size},
//             {1.f, 1.f, 1.f },
//             {1.f, 1.f, 1.f },
//             {1.f, 1.f, 1.f });
//         tls.back()->draw_rectangle_wo_normals(
//             center + FixedArray<float, 3>{0.f, -scale * veg_size, 0.f},
//             center + FixedArray<float, 3>{0.f, scale * veg_size, 0.f},
//             center + FixedArray<float, 3>{0.f, scale * veg_size, 2 * scale * veg_size},
//             center + FixedArray<float, 3>{0.f, -scale * veg_size, 2 * scale * veg_size},
//             {1.f, 1.f, 1.f },
//             {1.f, 1.f, 1.f },
//             {1.f, 1.f, 1.f });
//     }
// }

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

void Mlib::colorize_height_map(std::list<FixedArray<ColoredVertex<CompressedScenePos>, 3>>& triangles)
{
    StaticFaceLighting sfl{ true }; // true == swap_yz
    for (auto& t : triangles) {
        t(0).color = Colors::from_rgb(sfl.get_color(Colors::to_rgb(t(0).color), t(0).normal));
        t(1).color = Colors::from_rgb(sfl.get_color(Colors::to_rgb(t(1).color), t(1).normal));
        t(2).color = Colors::from_rgb(sfl.get_color(Colors::to_rgb(t(2).color), t(2).normal));
    }
}

template <class TContainer, class TGetOrderableFixedArray>
void to_orderable_fixed_array(
    TContainer& result,
    const TContainer& nodes,
    bool verbose,
    const TGetOrderableFixedArray& get_orderable_fixed_array)
{
    typedef decltype(get_orderable_fixed_array(nodes.front())) Key;
    std::set<Key> pts;
    for (const auto& p : nodes) {
        auto o = get_orderable_fixed_array(p);
        if (pts.find(o) != pts.end()) {
            if (verbose) {
                lerr() << "Removing duplicate point " << o;
            }
        } else {
            pts.insert(o);
            result.push_back(p);
        }
    }
}

UUVector<FixedArray<CompressedScenePos, 2>> Mlib::removed_duplicates(
    const UUVector<FixedArray<CompressedScenePos, 2>>& nodes,
    bool verbose)
{
    UUVector<FixedArray<CompressedScenePos, 2>> result;
    result.reserve(nodes.size());
    to_orderable_fixed_array(
        result,
        nodes,
        verbose,
        [](const FixedArray<CompressedScenePos, 2>& p){return OrderableFixedArray<CompressedScenePos, 2>{p};});
    return result;
}

std::list<FixedArray<CompressedScenePos, 2>> Mlib::removed_duplicates(
    const std::list<FixedArray<CompressedScenePos, 2>>& nodes,
    bool verbose)
{
    std::list<FixedArray<CompressedScenePos, 2>> result;
    to_orderable_fixed_array(
        result,
        nodes,
        verbose,
        [](const FixedArray<CompressedScenePos, 2>& p){return OrderableFixedArray<CompressedScenePos, 2>{p};});
    return result;
}

std::list<SteinerPointInfo> Mlib::removed_duplicates(
    const std::list<SteinerPointInfo>& nodes,
    bool verbose)
{
    std::list<SteinerPointInfo> result;
    to_orderable_fixed_array(
        result,
        nodes,
        verbose,
        [](const SteinerPointInfo& p){return OrderableFixedArray<CompressedScenePos, 3>{p.position};});
    return result;
}

void Mlib::check_curb_validity(float curb_alpha, float curb2_alpha) {
    if (curb_alpha > curb2_alpha) {
        THROW_OR_ABORT("curb_alpha > curb2_alpha");
    }
    if (curb_alpha <= 0) {
        THROW_OR_ABORT("curb_alpha <= 0");
    }
    if (curb2_alpha > 1) {
        THROW_OR_ABORT("curb2_alpha > 1");
    }
}

template float Mlib::parse_meters<float>(
    const std::map<std::string, std::string>& tags,
    const std::string& key,
    float default_value);

template double Mlib::parse_meters<double>(
    const std::map<std::string, std::string>& tags,
    const std::string& key,
    double default_value);
