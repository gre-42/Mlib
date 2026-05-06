#include "Gpu_Renderable_Colored_Vertex_Array.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Primitives/Extremal_Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Geometry/Primitives/Extremal_Bounding_Sphere.hpp>
#include <Mlib/OpenGL/CHK.hpp>
#include <Mlib/OpenGL/Instance_Handles/IArray_Buffer.hpp>
#include <Mlib/OpenGL/Resources/Colored_Vertex_Array_Resource/Shader_Structs.hpp>
#include <Mlib/Scene_Graph/Render/Attribute_Index_Calculator.hpp>
#include <Mlib/Scene_Graph/Render/IGpu_Instance_Buffers.hpp>
#include <Mlib/Scene_Graph/Render/IGpu_Vertex_Data.hpp>
#include <ranges>

using namespace Mlib;

GpuRenderableColoredVertexArray::GpuRenderableColoredVertexArray(
    std::shared_ptr<IGpuVertexData> vertices,
    std::shared_ptr<IGpuInstanceBuffers> instances)
    : vertices_{ std::move(vertices) }
    , instances_{ std::move(instances) }
{
    // va_ is initialized in the "initialize()" method
}

void GpuRenderableColoredVertexArray::update_legacy() {
    va_.update();
}

void GpuRenderableColoredVertexArray::bind() const {
    va_.bind();
}

bool GpuRenderableColoredVertexArray::copy_in_progress() const {
    return va_.copy_in_progress();
}

bool GpuRenderableColoredVertexArray::initialized() const {
    return va_.initialized();
}

void GpuRenderableColoredVertexArray::initialize() {
    vertices_->visit_buffers([this](IArrayBuffer& buffer){
        va_.add_array_buffer(buffer);
        return true;
    });
    if (instances_ != nullptr) {
        va_.set_instance_buffer(*instances_);
    }
    // https://stackoverflow.com/a/13405205/2292832
    va_.initialize();

    auto attr_idc = get_attribute_index_calculator(*this);
    auto attr_ids = attr_idc.build();

    {
        vertices_->vertex_buffer().bind();
        ColoredVertex<float>* cv = nullptr;
        CHK(glEnableVertexAttribArray(attr_ids.idx_position));
        CHK(glVertexAttribPointer(attr_ids.idx_position, 3, GL_FLOAT, GL_FALSE, sizeof(ColoredVertex<float>), &cv->position));
        CHK(glEnableVertexAttribArray(attr_ids.idx_color));
        CHK(glVertexAttribPointer(attr_ids.idx_color, 3, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ColoredVertex<float>), &cv->color));
        CHK(glEnableVertexAttribArray(attr_ids.idx_uv_0));
        CHK(glVertexAttribPointer(attr_ids.idx_uv_0, 2, GL_FLOAT, GL_FALSE, sizeof(ColoredVertex<float>), &cv->uv));
        // The vertex array is cached by cva => Use material properties, not the RenderProgramIdentifier.
        if (attr_idc.has_normal) {
            CHK(glEnableVertexAttribArray(attr_ids.idx_normal));
            CHK(glVertexAttribPointer(attr_ids.idx_normal, 3, GL_FLOAT, GL_FALSE, sizeof(ColoredVertex<float>), &cv->normal));
        }
        // The vertex array is cached by cva => Use material properties, not the RenderProgramIdentifier.
        if (attr_idc.has_tangent) {
            CHK(glEnableVertexAttribArray(attr_ids.idx_tangent));
            CHK(glVertexAttribPointer(attr_ids.idx_tangent, 3, GL_FLOAT, GL_FALSE, sizeof(ColoredVertex<float>), &cv->tangent));
        }
    }
    if (vertices_->nuvs() > 1) {
        if (vertices_->nuvs() != attr_ids.uv_count) {
            throw std::runtime_error("Unexpected number of UV indices");
        }
        for (auto i : std::views::iota(0zu, vertices_->nuvs() - 1)) {
            vertices_->uv1_buffer(i).bind();
            CHK(glEnableVertexAttribArray(integral_cast<GLuint>(attr_ids.idx_uv_1 + i)));
            CHK(glVertexAttribPointer(integral_cast<GLuint>(attr_ids.idx_uv_1 + i), 2, GL_FLOAT, GL_FALSE, sizeof(FixedArray<float, 2>), nullptr));
        }
    }
    {
        if (vertices_->ncweights() != attr_ids.cweight_count) {
            throw std::runtime_error("Unexpected number of weight coefficients");
        }
        for (auto i : std::views::iota(0zu, vertices_->ncweights())) {
            vertices_->cweight_buffer(i).bind();
            CHK(glEnableVertexAttribArray(integral_cast<GLuint>(attr_ids.idx_cweight_0 + i)));
            CHK(glVertexAttribPointer(integral_cast<GLuint>(attr_ids.idx_cweight_0 + i), 1, GL_FLOAT, GL_FALSE, sizeof(float), nullptr));
        }
    }
    if (vertices_->has_alpha()) {
        vertices_->alpha_buffer().bind();
        CHK(glEnableVertexAttribArray(integral_cast<GLuint>(attr_ids.idx_alpha)));
        CHK(glVertexAttribPointer(integral_cast<GLuint>(attr_ids.idx_alpha), 1, GL_FLOAT, GL_FALSE, sizeof(float), nullptr));
    }
    if (vertices_->has_bone_indices()) {
        vertices_->bone_weight_buffer().bind();
        ShaderBoneWeight* bw = nullptr;
        CHK(glEnableVertexAttribArray(attr_ids.idx_bone_indices));
        CHK(glVertexAttribIPointer(attr_ids.idx_bone_indices, ANIMATION_NINTERPOLATED, GL_UNSIGNED_BYTE, sizeof(ShaderBoneWeight), &bw->indices));
        CHK(glEnableVertexAttribArray(attr_ids.idx_bone_weights));
        CHK(glVertexAttribPointer(attr_ids.idx_bone_weights, ANIMATION_NINTERPOLATED, GL_FLOAT, GL_FALSE, sizeof(ShaderBoneWeight), &bw->weights));
    }
    if (vertices_->has_continuous_triangle_texture_layers() &&
        vertices_->has_discrete_triangle_texture_layers())
    {
        throw std::runtime_error("Detected both, discrete and continuous texture layers");
    }
    if (vertices_->has_continuous_triangle_texture_layers()) {
        vertices_->texture_layer_buffer().bind();
        CHK(glEnableVertexAttribArray(attr_ids.idx_texture_layer));
        CHK(glVertexAttribPointer(attr_ids.idx_texture_layer, 1, GL_FLOAT, GL_FALSE, sizeof(float), nullptr));
    }
    if (vertices_->has_discrete_triangle_texture_layers()) {
        vertices_->texture_layer_buffer().bind();
        CHK(glEnableVertexAttribArray(attr_ids.idx_texture_layer));
        CHK(glVertexAttribIPointer(attr_ids.idx_texture_layer, 1, GL_UNSIGNED_BYTE, sizeof(uint8_t), nullptr));
    }
    if (vertices_->has_interiormap()) {
        vertices_->interior_mapping_buffer().bind();
        ShaderInteriorMappedFacade* im = nullptr;
        CHK(glEnableVertexAttribArray(attr_ids.idx_interior_mapping_bottom_left));
        CHK(glVertexAttribPointer(attr_ids.idx_interior_mapping_bottom_left, 3, GL_FLOAT, GL_FALSE, sizeof(ShaderInteriorMappedFacade), &im->bottom_left));
        CHK(glEnableVertexAttribArray(attr_ids.idx_interior_mapping_uvmap));
        CHK(glVertexAttribPointer(attr_ids.idx_interior_mapping_uvmap, 4, GL_FLOAT, GL_FALSE, sizeof(ShaderInteriorMappedFacade), &im->uvmap));
    }

    if (instances_ != nullptr) {
        instances_->bind(
            attr_ids.idx_instance_attrs,
            attr_ids.idx_rotation_quaternion,
            attr_ids.idx_billboard_ids,
            attr_ids.idx_texture_layer);
    }

    CHK(glBindVertexArray(0));
}

void GpuRenderableColoredVertexArray::wait() const {
    va_.wait();
}

std::shared_ptr<IGpuVertexData> GpuRenderableColoredVertexArray::vertices() {
    return vertices_;
}

std::shared_ptr<IGpuInstanceBuffers> GpuRenderableColoredVertexArray::instances() {
    return instances_;
}

std::shared_ptr<const IGpuVertexData> GpuRenderableColoredVertexArray::vertices() const {
    return vertices_;
}

std::shared_ptr<const IGpuInstanceBuffers> GpuRenderableColoredVertexArray::instances() const {
    return instances_;
}

const ExtremalAxisAlignedBoundingBox<float, 3>& GpuRenderableColoredVertexArray::aabb() const {
    if (instances_ == nullptr) {
        return vertices_->aabb();
    } else {
        static auto result = ExtremalAxisAlignedBoundingBox<float, 3>{ExtremalBoundingVolume::FULL};
        return result;
    }
}

const ExtremalBoundingSphere<float, 3>& GpuRenderableColoredVertexArray::bounding_sphere() const {
    if (instances_ == nullptr) {
        return vertices_->bounding_sphere();
    } else {
        static auto result = ExtremalBoundingSphere<float, 3>{ExtremalBoundingVolume::FULL};
        return result;
    }
}

void GpuRenderableColoredVertexArray::extend_aabb(ExtremalAxisAlignedBoundingBox<CompressedScenePos, 3>& aabb) const {
    if (instances_ == nullptr) {
        vertices_->extend_aabb(aabb);
    } else {
        aabb = ExtremalBoundingVolume::FULL;
    }
}

void GpuRenderableColoredVertexArray::extend_bounding_sphere(ExtremalBoundingSphere<CompressedScenePos, 3>& bounding_sphere) const {
    if (instances_ == nullptr) {
        vertices_->extend_bounding_sphere(bounding_sphere);
    } else {
        bounding_sphere = ExtremalBoundingVolume::FULL;
    }
}

void GpuRenderableColoredVertexArray::extend_aabb(
    const TransformationMatrix<SceneDir, ScenePos, 3>& mv,
    AxisAlignedBoundingBox<CompressedScenePos, 3>& aabb) const
{
    if (instances_ == nullptr) {
        vertices_->extend_aabb(mv, aabb);
    } else {
        throw std::runtime_error("extend_aabb called on an array with instances");
    }
}

ScenePos GpuRenderableColoredVertexArray::max_center_distance2(BillboardId billboard_id) const {
    if (instances_ == nullptr) {
        return vertices_->max_center_distance2(billboard_id);
    } else {
        throw std::runtime_error("max_center_distance2 called on an array with instances");
    }
}

std::string GpuRenderableColoredVertexArray::identifier() const {
    if (instances_ != nullptr) {
        return vertices_->identifier() + " (instances)";
    } else {
        return vertices_->identifier();
    }
}

void GpuRenderableColoredVertexArray::print_stats(std::ostream& ostr) const {
    vertices_->print_stats(ostr);
    if (instances_ != nullptr) {
        instances_->print_stats(ostr);
    }
}
