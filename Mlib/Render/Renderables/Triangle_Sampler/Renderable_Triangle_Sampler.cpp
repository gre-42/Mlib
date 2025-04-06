#include "Renderable_Triangle_Sampler.hpp"
#include <Mlib/Geometry/Intersection/Bounding_Sphere.hpp>
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Transformed_Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Geometry/Mesh/Triangle_Sampler2.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Math/Transformation/Translation_Matrix.hpp>
#include <Mlib/Render/Renderables/Triangle_Sampler/Resource_Name_Cycle.hpp>
#include <Mlib/Render/Renderables/Triangle_Sampler/Sample_Triangle_Interior_Instances.hpp>
#include <Mlib/Render/Renderables/Triangle_Sampler/Terrain_Style.hpp>
#include <Mlib/Render/Renderables/Triangle_Sampler/Terrain_Styles.hpp>
#include <Mlib/Render/Renderables/Triangle_Sampler/Terrain_Type.hpp>
#include <Mlib/Render/Yield.hpp>
#include <Mlib/Scene_Graph/Culling/Visibility_Check.hpp>
#include <Mlib/Scene_Graph/Elements/Rendering_Strategies.hpp>
#include <Mlib/Scene_Graph/Instances/Small_Instances_Queues.hpp>
#include <Mlib/Scene_Graph/Resources/Parsed_Resource_Name.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Scene_Graph/Scene_Graph_Config.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

RenderableTriangleSampler::RenderableTriangleSampler(
    const SceneNodeResources& scene_node_resources,
    const TerrainStyles& terrain_styles,
    const TerrainTriangles& terrain_triangles,
    const std::list<const UUList<FixedArray<ColoredVertex<CompressedScenePos>, 3>>*>& no_grass,
    const Bvh<CompressedScenePos, 3, FixedArray<CompressedScenePos, 3, 3>>* street_bvh,
    ScenePos scale,
    UpAxis up_axis)
    : scene_node_resources_{scene_node_resources}
    , terrain_styles_{terrain_styles}
    , terrain_triangles_{terrain_triangles}
    , no_grass_{no_grass}
    , street_bvh_{street_bvh}
    , scale_{scale}
    , up_axis_{up_axis}
{}

RenderableTriangleSampler::~RenderableTriangleSampler()
{}

PhysicsMaterial RenderableTriangleSampler::physics_attributes() const {
    return PhysicsMaterial::ATTR_VISIBLE;
}

RenderingStrategies RenderableTriangleSampler::rendering_strategies() const {
    return RenderingStrategies::INSTANCES_SORTED_CONTINUOUSLY;
}

bool RenderableTriangleSampler::requires_render_pass(ExternalRenderPassType render_pass) const
{
    return false;
}

bool RenderableTriangleSampler::requires_blending_pass(ExternalRenderPassType render_pass) const
{
    return false;
}

void RenderableTriangleSampler::append_sorted_instances_to_queue(
    const FixedArray<ScenePos, 4, 4>& mvp,
    const TransformationMatrix<float, ScenePos, 3>& m,
    const TransformationMatrix<float, ScenePos, 3>& iv,
    const FixedArray<ScenePos, 3>& offset,
    BillboardId billboard_id,
    const SceneGraphConfig& scene_graph_config,
    SmallInstancesQueues& instances_queue) const
{
    unsigned int yield_counter = 0;
    bool orthographic = VisibilityCheck{ mvp }.orthographic();
    auto sample_triangles = [&](
        const Bvh<CompressedScenePos, 3, TriangleAndSeed>& triangle_bvh,
        const TerrainStyle& terrain_style,
        const Bvh<CompressedScenePos, 3, FixedArray<CompressedScenePos, 3, 3>>* boundary_bvh)
    {
        ScenePos max_distance_to_camera = terrain_style.max_distance_to_camera(scene_node_resources_);

        TriangleInteriorInstancesSampler tiis{
            terrain_style,
            scale_,
            up_axis_,
            boundary_bvh,
            terrain_style.foliagemap(),
            terrain_style.config.foliagemap_scale,
            terrain_style.mudmap()};
        auto traverse_triangle = [&](const TriangleAndSeed& t){
            if (!orthographic) {
                BoundingSphere<ScenePos, 3> bs{funpack(FixedArray<CompressedScenePos, 3, 3>{
                    t.triangle(0).position,
                    t.triangle(1).position,
                    t.triangle(2).position})};
                TranslationMatrix<ScenePos, 3> mc_rel{ bs.center };
                auto mvp_center = mvp * mc_rel;
                if (!VisibilityCheck{ mvp_center }.is_visible(ScenePos(2) * bs.radius / scale_ + max_distance_to_camera)) {
                    return;
                }
            }
            tiis.sample_triangle(
                t.triangle,
                t.seed,
                [this, &mvp, &m, &offset, &scene_graph_config, &instances_queue, &yield_counter](
                    const FixedArray<CompressedScenePos, 3>& p,
                    const ParsedResourceName& prn)
                {
                    if (++yield_counter >= THREAD_YIELD_INTERVAL) {
                        yield_counter = 0;
                        std::this_thread::yield();
                    };
                    auto scvas = scene_node_resources_.get_single_precision_arrays(*prn.name);
                    TranslationMatrix<ScenePos, 3> mi_rel{ funpack(p) };
                    auto mvp_instance = mvp * mi_rel;
                    auto m_instance_d = m * mi_rel;
                    instances_queue.insert(
                        scvas,
                        mvp_instance,
                        m_instance_d,
                        offset,
                        prn.billboard_id,
                        scene_graph_config);
                });
        };
        if (orthographic) {
            if (terrain_style.config.size_classification == SizeClassification::LARGE) {
                triangle_bvh.visit_all([&traverse_triangle](const auto& d){
                    traverse_triangle(d.payload());
                    return true;
                });
            }
        } else {
            auto rel_camera_position = m.inverted_scaled().transform(iv.t);
            triangle_bvh.visit(
                AxisAlignedBoundingBox<ScenePos, 3>::from_center_and_radius(rel_camera_position, max_distance_to_camera * scale_)
                .casted<CompressedScenePos>(),
                [&traverse_triangle](const TriangleAndSeed& t){
                    traverse_triangle(t);
                    return true;
                });
        }
    };
    auto add_triangles = [this](
        std::map<const TerrainStyle*, Bvh<CompressedScenePos, 3, TriangleAndSeed>>& bvhs,
        const TerrainStyle& terrain_style,
        const UUList<FixedArray<ColoredVertex<CompressedScenePos>, 3>>& gtl)
    {
        auto it = bvhs.find(&terrain_style);
        if (it == bvhs.end()) {
            auto ins = bvhs.try_emplace(&terrain_style, fixed_full<CompressedScenePos, 3>(CompressedScenePos(0.3 * scale_)), 17);
            if (!ins.second) {
                verbose_abort("Internal error, could not insert BVH");
            }
            it = ins.first;
        }
        unsigned int seed = 0;
        for (const auto& t : gtl) {
            ++seed;
            auto aabb = AxisAlignedBoundingBox<CompressedScenePos, 3>::from_points(FixedArray<CompressedScenePos, 3, 3>{
                t(0).position,
                t(1).position,
                t(2).position});
            it->second.insert(aabb, TriangleAndSeed{.triangle = t, .seed = seed});
        }
    };
    if (!grass_bvhs_.has_value()) {
        grass_bvhs_.emplace();
        if (const auto& style = terrain_styles_.near_grass_terrain_style; style.is_visible()) {
            if (auto tris = terrain_triangles_.grass; tris != nullptr) {
                add_triangles(*grass_bvhs_, style, *tris);
            }
            if (auto tris = terrain_triangles_.elevated_grass; tris != nullptr) {
                add_triangles(*grass_bvhs_, style, *tris);
            }
        }
        if (const auto& style = terrain_styles_.near_wayside1_grass_terrain_style; style.is_visible()) {
            if (auto tris = terrain_triangles_.wayside1_grass; tris != nullptr) {
                add_triangles(*grass_bvhs_, style, *tris);
            }
        }
        if (const auto& style = terrain_styles_.near_wayside2_grass_terrain_style; style.is_visible()) {
            if (auto tris = terrain_triangles_.wayside2_grass; tris != nullptr) {
                add_triangles(*grass_bvhs_, style, *tris);
            }
        }
        if (const auto& style = terrain_styles_.near_flowers_terrain_style; style.is_visible()) {
            if (auto tris = terrain_triangles_.flowers; tris != nullptr) {
                add_triangles(*grass_bvhs_, style, *tris);
            }
        }
        if (const auto& style = terrain_styles_.near_trees_terrain_style; style.is_visible()) {
            if (auto tris = terrain_triangles_.trees; tris != nullptr) {
                add_triangles(*grass_bvhs_, style, *tris);
            }
        }
        if (const auto& style = terrain_styles_.street_mud_terrain_style; style.is_visible()) {
            if (auto tris = terrain_triangles_.street_mud_grass; tris != nullptr) {
                add_triangles(*grass_bvhs_, style, *tris);
            }
        }
        if (const auto& style = terrain_styles_.path_mud_terrain_style; style.is_visible()) {
            if (auto tris = terrain_triangles_.path_mud_grass; tris != nullptr) {
                add_triangles(*grass_bvhs_, style, *tris);
            }
        }
    }
    for (const auto& [style, bvh] : grass_bvhs_.value()) {
        sample_triangles(
            bvh,
            *style,
            street_bvh_);
    }
    if (!no_grass_bvhs_.has_value()) {
        no_grass_bvhs_.emplace();
        if (terrain_styles_.no_grass_decals_terrain_style.is_visible()) {
            for (const auto& lst : no_grass_) {
                add_triangles(*no_grass_bvhs_, terrain_styles_.no_grass_decals_terrain_style, *lst);
            }
        }
    }
    for (const auto& [style, bvh] : no_grass_bvhs_.value()) {
        sample_triangles(
            bvh,
            *style,
            nullptr);
    }
}

void RenderableTriangleSampler::extend_aabb(
    const TransformationMatrix<float, ScenePos, 3>& mv,
    ExternalRenderPassType render_pass,
    AxisAlignedBoundingBox<CompressedScenePos, 3>& aabb) const
{
    // Not yet implemented
}

ScenePos RenderableTriangleSampler::max_center_distance(BillboardId billboard_id) const {
    return INFINITY;
}
