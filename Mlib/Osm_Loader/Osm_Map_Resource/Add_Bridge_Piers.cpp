#include "Add_Bridge_Piers.hpp"
#include <Mlib/Geometry/Intersection/Interval.hpp>
#include <Mlib/Geometry/Material.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array_Filter.hpp>
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Geometry/Morphology.hpp>
#include <Mlib/Math/Fixed_Rotation_2D.hpp>
#include <Mlib/Math/Lerp.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Ground_Bvh.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <Mlib/Scene_Graph/Resources/Parsed_Resource_Name.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>

using namespace Mlib;

void Mlib::add_bridge_piers(
    std::list<std::shared_ptr<TriangleList<CompressedScenePos>>>& tls_bridge_piers,
    Material material,
    const Morphology& morphology,
    SceneDir bridge_pier_radius,
    const SceneNodeResources& scene_node_resources,
    const VariableAndHash<std::string>& model_name,
    const GroundBvh& ground_bvh,
    const std::map<std::string, Node>& nodes,
    const std::map<std::string, Way>& ways)
{
    PointWithoutPayloadVectorBvh<CompressedScenePos, 2> pier_bvh{
        FixedArray<CompressedScenePos, 2>{(CompressedScenePos)0.1f, (CompressedScenePos)0.1f},
        17 };
    auto model_triangles = scene_node_resources.get_single_precision_arrays(model_name, ColoredVertexArrayFilter{});
    auto& res = *tls_bridge_piers.emplace_back(
        std::make_shared<TriangleList<CompressedScenePos>>(
            "bridge_piers",
            material,
            morphology
    ));
    for (const auto& [_, w] : ways) {
        for (const auto& node_id : w.nd) {
            const auto& node = nodes.at(node_id);
            if (!pier_bvh.has_neighbor2(node.position, (CompressedScenePos)10.f, [&](const auto& n) {
                return sum(squared(node.position - n));
            }))
            {
                auto in = ground_bvh.bridge_gap(node.position);
                if ((in.min == GroundBvh::MIN_BRIDGE_GROUND) ||
                    (in.max == GroundBvh::MAX_BRIDGE_AIR))
                {
                    continue;
                }
                if (in.length() > (CompressedScenePos)2.f) {
                    auto trafo = [&](){
                        SceneDir yangle = 0.f;
                        auto get_height = [&](ScenePos zr) {
                            return lerp11(in.min, in.max, zr);
                        };
                        auto w = get_height(0.f);
                        auto z = (SceneDir)(get_height(1.f) - w);
                        auto R2 = bridge_pier_radius * fixed_rotation_2d(yangle);
                        auto R = FixedArray<SceneDir, 3, 3>::init(
                            R2(0, 0), R2(0, 1), 0.f,
                            R2(1, 0), R2(1, 1), 0.f,
                            0.f, 0.f, z);
                        return TransformationMatrix<SceneDir, ScenePos, 3>{
                            R,
                            FixedArray<ScenePos, 3>{funpack(node.position(0)), funpack(node.position(1)), w}};
                    }();
                    float uv_scale_y = 1.f;
                    auto uv_scale = FixedArray<float, 2>{1.f, uv_scale_y * (float)in.length()};
                    for (const auto& cva : model_triangles) {
                        if (cva->name.name() != "bridge_pier") {
                            THROW_OR_ABORT(
                                (std::stringstream() << "Object name should be \"bridge_pier\", but is \"" <<
                                    cva->name.name() << '"').str());
                        }
                        if (!cva->quads.empty()) {
                            THROW_OR_ABORT(
                                (std::stringstream() << "Quads are not supported for bridge piers: \"" <<
                                    cva->name.name() << '"').str());
                        }
                        for (const auto& t : cva->triangles) {
                            for (size_t i = 0; i < 3; ++i) {
                                if (std::abs(t(i).position(2)) != 1.f) {
                                    THROW_OR_ABORT("Bridge pier z-coordinate is not +1 or -1");
                                }
                            }
                        }
                        auto tcva = cva->transformed<CompressedScenePos>(trafo, "_transformed");
                        for (const auto& t : tcva->triangles) {
                            res.draw_triangle_with_normals(
                                t(0).position,
                                t(1).position,
                                t(2).position,
                                t(0).normal,
                                t(1).normal,
                                t(2).normal,
                                t(0).color,
                                t(1).color,
                                t(2).color,
                                t(0).uv * uv_scale,
                                t(1).uv * uv_scale,
                                t(2).uv * uv_scale);
                        }
                    }
                    pier_bvh.insert(PointWithoutPayload{ node.position });
                }
            }
        }
    }
}
