#pragma once
#include <Mlib/Geometry/Billboard_Id.hpp>
#include <Mlib/Scene_Config/Scene_Precision.hpp>
#include <cstddef>
#include <functional>
#include <iosfwd>
#include <string>

namespace Mlib {

class VertexArray;
template <typename TData, size_t... tshape>
class FixedArray;
template <class TDir, class TPos, size_t n>
class TransformationMatrix;
class IArrayBuffer;
struct MeshMeta;
template <class TData, size_t tndim>
class ExtremalAxisAlignedBoundingBox;
template <class TData, size_t tndim>
class ExtremalBoundingSphere;
template <class TData, size_t tndim>
class AxisAlignedBoundingBox;

class IGpuVertexData {
public:
    virtual ~IGpuVertexData() = default;
    virtual size_t ntriangles() const = 0;
    virtual size_t nuvs() const = 0;
    virtual size_t ncweights() const = 0;
    virtual bool has_alpha() const = 0;
    virtual bool has_continuous_triangle_texture_layers() const = 0;
    virtual bool has_discrete_triangle_texture_layers() const = 0;
    virtual bool has_interiormap() const = 0;
    virtual bool has_bone_indices() const = 0;
    virtual IArrayBuffer& vertex_buffer() = 0;
    virtual IArrayBuffer& bone_weight_buffer() = 0;
    virtual IArrayBuffer& texture_layer_buffer() = 0;
    virtual IArrayBuffer& interior_mapping_buffer() = 0;
    virtual IArrayBuffer& uv1_buffer(size_t i) = 0;
    virtual IArrayBuffer& cweight_buffer(size_t i) = 0;
    virtual IArrayBuffer& alpha_buffer() = 0;
    virtual void delete_triangles_far_away_legacy(
        const FixedArray<float, 3>& position,
        const TransformationMatrix<float, float, 3>& m,
        float draw_distance_add,
        float draw_distance_slop,
        size_t noperations,
        bool run_in_background,
        bool is_static) = 0;
    virtual const MeshMeta& mesh_meta() const = 0;
    virtual const ExtremalAxisAlignedBoundingBox<float, 3>& aabb() const = 0;
    virtual const ExtremalBoundingSphere<float, 3>& bounding_sphere() const = 0;
    virtual void extend_aabb(ExtremalAxisAlignedBoundingBox<CompressedScenePos, 3>& aabb) const = 0;
    virtual void extend_bounding_sphere(ExtremalBoundingSphere<CompressedScenePos, 3>& bounding_sphere) const = 0;
    virtual void extend_aabb(
        const TransformationMatrix<SceneDir, ScenePos, 3>& mv,
        AxisAlignedBoundingBox<CompressedScenePos, 3>& aabb) const = 0;
    virtual ScenePos max_center_distance2(BillboardId billboard_id) const = 0;
    virtual std::string identifier() const = 0;
    virtual void print_stats(std::ostream& ostr) const = 0;
    bool visit_buffers(const std::function<bool(IArrayBuffer&)>& op);
    void wait();
    bool copy_in_progress();
};

}
