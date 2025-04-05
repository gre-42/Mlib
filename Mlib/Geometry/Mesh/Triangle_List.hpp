#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Default_Uninitialized_List.hpp>
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Material.hpp>
#include <Mlib/Geometry/Mesh/Bone_Weight.hpp>
#include <Mlib/Geometry/Modifier_Backlog.hpp>
#include <Mlib/Geometry/Morphology.hpp>
#include <Mlib/Geometry/Normal_Vector_Error_Behavior.hpp>
#include <Mlib/Geometry/Rectangle_Triangulation_Mode.hpp>
#include <Mlib/Geometry/Triangle_Tangent_Error_Behavior.hpp>
#include <Mlib/Strings/Group_And_Name.hpp>
#include <cereal/access.hpp>
#include <cstdint>
#include <list>
#include <memory>
#include <optional>
#include <set>
#include <unordered_map>
#include <vector>

namespace Mlib {

template <class TPos>
struct ColoredVertex;
template <class TPos>
class ColoredVertexArray;
template <class TPos>
class VertexHeightBinding;

template <class TPos>
class TriangleList {
    TriangleList() = delete;
    TriangleList(const TriangleList&) = delete;
    TriangleList& operator = (const TriangleList&) = delete;
public:
    TriangleList(
        GroupAndName name,
        const Material& material,
        const Morphology& morphology,
        UUList<FixedArray<ColoredVertex<TPos>, 4>>&& quads = {},
        UUList<FixedArray<ColoredVertex<TPos>, 3>>&& triangles = {},
        UUList<FixedArray<std::vector<BoneWeight>, 3>>&& triangle_bone_weights = {},
        UUVector<FixedArray<float, 4>>&& interiormap_uvmaps = {});
    void draw_triangle_with_normals(
        const FixedArray<TPos, 3>& p00,
        const FixedArray<TPos, 3>& p10,
        const FixedArray<TPos, 3>& p01,
        const FixedArray<float, 3>& n00,
        const FixedArray<float, 3>& n10,
        const FixedArray<float, 3>& n01,
        const FixedArray<uint8_t, 4>& c00 = Colors::RED,
        const FixedArray<uint8_t, 4>& c10 = Colors::GREEN,
        const FixedArray<uint8_t, 4>& c01 = Colors::BLUE,
        const FixedArray<float, 2>& u00 = {0.f, 0.f},
        const FixedArray<float, 2>& u10 = {1.f, 0.f},
        const FixedArray<float, 2>& u01 = {0.f, 1.f},
        const std::optional<FixedArray<float, 4>>& interiormap_uvmap = std::nullopt,
        const std::vector<BoneWeight>& b00 = {},
        const std::vector<BoneWeight>& b10 = {},
        const std::vector<BoneWeight>& b01 = {},
        TriangleTangentErrorBehavior tangent_error_behavior = TriangleTangentErrorBehavior::THROW,
        ColoredVertex<TPos>** pp00 = nullptr,
        ColoredVertex<TPos>** pp10 = nullptr,
        ColoredVertex<TPos>** pp01 = nullptr);
    void draw_triangle_wo_normals(
        const FixedArray<TPos, 3>& p00,
        const FixedArray<TPos, 3>& p10,
        const FixedArray<TPos, 3>& p01,
        const FixedArray<uint8_t, 4>& c00 = Colors::RED,
        const FixedArray<uint8_t, 4>& c10 = Colors::GREEN,
        const FixedArray<uint8_t, 4>& c01 = Colors::BLUE,
        const FixedArray<float, 2>& u00 = {0.f, 0.f},
        const FixedArray<float, 2>& u10 = {1.f, 0.f},
        const FixedArray<float, 2>& u01 = {0.f, 1.f},
        const std::optional<FixedArray<float, 4>>& interiormap_uvmap = std::nullopt,
        const std::vector<BoneWeight>& b00 = {},
        const std::vector<BoneWeight>& b10 = {},
        const std::vector<BoneWeight>& b01 = {},
        NormalVectorErrorBehavior normal_error_behavior = NormalVectorErrorBehavior::THROW,
        TriangleTangentErrorBehavior tangent_error_behavior = TriangleTangentErrorBehavior::THROW,
        ColoredVertex<TPos>** pp00 = nullptr,
        ColoredVertex<TPos>** pp10 = nullptr,
        ColoredVertex<TPos>** pp01 = nullptr);
    void draw_rectangle_with_normals(
        const FixedArray<TPos, 3>& p00,
        const FixedArray<TPos, 3>& p10,
        const FixedArray<TPos, 3>& p11,
        const FixedArray<TPos, 3>& p01,
        const FixedArray<float, 3>& n00,
        const FixedArray<float, 3>& n10,
        const FixedArray<float, 3>& n11,
        const FixedArray<float, 3>& n01,
        const FixedArray<uint8_t, 4>& c00 = Colors::RED,
        const FixedArray<uint8_t, 4>& c10 = Colors::GREEN,
        const FixedArray<uint8_t, 4>& c11 = Colors::BLUE,
        const FixedArray<uint8_t, 4>& c01 = Colors::CYAN,
        const FixedArray<float, 2>& u00 = {0.f, 0.f},
        const FixedArray<float, 2>& u10 = {1.f, 0.f},
        const FixedArray<float, 2>& u11 = {1.f, 1.f},
        const FixedArray<float, 2>& u01 = {0.f, 1.f},
        const std::optional<FixedArray<float, 4>>& interiormap_uvmap = std::nullopt,
        const std::vector<BoneWeight>& b00 = {},
        const std::vector<BoneWeight>& b10 = {},
        const std::vector<BoneWeight>& b11 = {},
        const std::vector<BoneWeight>& b01 = {},
        TriangleTangentErrorBehavior tangent_error_behavior = TriangleTangentErrorBehavior::THROW,
        RectangleTriangulationMode rectangle_triangulation_mode = RectangleTriangulationMode::FIRST,
        ColoredVertex<TPos>** pp00a = nullptr,
        ColoredVertex<TPos>** pp11a = nullptr,
        ColoredVertex<TPos>** pp01a = nullptr,
        ColoredVertex<TPos>** pp00b = nullptr,
        ColoredVertex<TPos>** pp10b = nullptr,
        ColoredVertex<TPos>** pp11b = nullptr);
    void draw_rectangle_wo_normals(
        const FixedArray<TPos, 3>& p00,
        const FixedArray<TPos, 3>& p10,
        const FixedArray<TPos, 3>& p11,
        const FixedArray<TPos, 3>& p01,
        const FixedArray<uint8_t, 4>& c00 = Colors::RED,
        const FixedArray<uint8_t, 4>& c10 = Colors::GREEN,
        const FixedArray<uint8_t, 4>& c11 = Colors::BLUE,
        const FixedArray<uint8_t, 4>& c01 = Colors::CYAN,
        const FixedArray<float, 2>& u00 = {0.f, 0.f},
        const FixedArray<float, 2>& u10 = {1.f, 0.f},
        const FixedArray<float, 2>& u11 = {1.f, 1.f},
        const FixedArray<float, 2>& u01 = {0.f, 1.f},
        const std::optional<FixedArray<float, 4>>& interiormap_uvmap = std::nullopt,
        const std::vector<BoneWeight>& b00 = {},
        const std::vector<BoneWeight>& b10 = {},
        const std::vector<BoneWeight>& b11 = {},
        const std::vector<BoneWeight>& b01 = {},
        NormalVectorErrorBehavior normal_error_behavior = NormalVectorErrorBehavior::THROW,
        TriangleTangentErrorBehavior tangent_error_behavior = TriangleTangentErrorBehavior::THROW,
        RectangleTriangulationMode rectangle_triangulation_mode = RectangleTriangulationMode::FIRST,
        ColoredVertex<TPos>** pp00a = nullptr,
        ColoredVertex<TPos>** pp11a = nullptr,
        ColoredVertex<TPos>** pp01a = nullptr,
        ColoredVertex<TPos>** pp00b = nullptr,
        ColoredVertex<TPos>** pp10b = nullptr,
        ColoredVertex<TPos>** pp11b = nullptr);
    static void extrude(
        TriangleList& dest,
        const std::list<std::shared_ptr<TriangleList>>& triangle_lists,
        const std::list<std::shared_ptr<TriangleList>>* follower_triangles,
        const std::list<std::shared_ptr<TriangleList>>* source_triangles,
        const std::set<OrderableFixedArray<TPos, 3>>* clamped_vertices,
        const std::set<OrderableFixedArray<TPos, 3>>* vertices_not_to_connect,
        TPos height,
        float scale,
        float uv_scale_x,
        float uv_scale_y,
        bool uvs_equal_lengths,
        float ambient_occlusion);
    static std::list<std::shared_ptr<TriangleList>> concatenated(
        const std::list<std::shared_ptr<TriangleList>>& a,
        const std::list<std::shared_ptr<TriangleList>>& b);
    void delete_backfacing_triangles(std::list<FixedArray<ColoredVertex<TPos>, 3>>* deleted_triangles = nullptr);
    void calculate_triangle_normals(NormalVectorErrorBehavior error_behavior = NormalVectorErrorBehavior::THROW);
    void convert_triangle_to_vertex_normals();
    void flip();
    static void convert_triangle_to_vertex_normals(const std::list<std::shared_ptr<TriangleList>>& triangle_lists);
    static void ambient_occlusion_by_curvature(
        const std::list<std::shared_ptr<TriangleList>>& triangle_lists,
        float strength);
    static void smoothen_edges(
        std::unordered_map<FixedArray<TPos, 3>*, VertexHeightBinding<TPos>>& vertex_height_bindings,
        const std::unordered_map<OrderableFixedArray<TPos, 3>, FixedArray<float, 3>>& bias,
        const std::list<std::shared_ptr<TriangleList>>& edge_triangle_lists,
        const std::list<std::shared_ptr<TriangleList>>& excluded_triangle_lists,
        const std::list<std::shared_ptr<TriangleList>>& move_only_z_triangle_lists,
        const std::list<FixedArray<TPos, 3>*>& smoothed_vertices,
        float smoothness,
        size_t niterations,
        bool move_only_z = false,
        float decay = 0.97f);
    std::shared_ptr<ColoredVertexArray<TPos>> triangle_array() const;
    template <class Archive>
    void serialize(Archive& archive) {
        archive(name);
        archive(material);
        archive(morphology);
        archive(quads);
        archive(triangles);
        archive(triangle_bone_weights);
        archive(interiormap_uvmaps);
    }
    // From: https://github.com/USCiLab/cereal/issues/102
    template<typename Archive>
    static void load_and_construct(
        Archive& archive,
        cereal::construct<TriangleList>& construct)
    {
        GroupAndName name;
        Material material;
        Morphology morphology;
        UUList<FixedArray<ColoredVertex<TPos>, 4>> quads;
        UUList<FixedArray<ColoredVertex<TPos>, 3>> triangles;
        UUList<FixedArray<std::vector<BoneWeight>, 3>> triangle_bone_weights;
        UUVector<FixedArray<float, 4>> interiormap_uvmaps;

        archive(name);
        archive(material);
        archive(morphology);
        archive(quads);
        archive(triangles);
        archive(triangle_bone_weights);
        archive(interiormap_uvmaps);

        construct(
            name,
            material,
            morphology,
            std::move(quads),
            std::move(triangles),
            std::move(triangle_bone_weights),
            std::move(interiormap_uvmaps));
    }
    GroupAndName name;
    Material material;
    Morphology morphology;
    ModifierBacklog modifier_backlog;
    UUList<FixedArray<ColoredVertex<TPos>, 4>> quads;
    UUList<FixedArray<ColoredVertex<TPos>, 3>> triangles;
    UUList<FixedArray<std::vector<BoneWeight>, 3>> triangle_bone_weights;
    UUVector<FixedArray<float, 4>> interiormap_uvmaps;
};

}
