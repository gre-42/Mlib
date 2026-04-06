
#include "IGpu_Vertex_Data.hpp"
#include <Mlib/OpenGL/Instance_Handles/IArray_Buffer.hpp>

using namespace Mlib;

bool IGpuVertexData::visit_buffers(const std::function<bool(IArrayBuffer&)>& op) {
    if (!op(vertex_buffer())) return false;
    if (has_bone_indices()) {
        if (!op(bone_weight_buffer())) return false;
    }
    if (has_continuous_triangle_texture_layers() || has_discrete_triangle_texture_layers()) {
        if (!op(texture_layer_buffer())) return false;
    }
    if (has_interiormap()) {
        if (!op(interior_mapping_buffer())) return false;
    }
    for (size_t i = 0; i < nuvs() - 1; ++i) {
        if (!op(uv1_buffer(i))) return false;
    }
    for (size_t i = 0; i < ncweights(); ++i) {
        if (!op(cweight_buffer(i))) return false;
    }
    if (has_alpha()) {
        if (!op(alpha_buffer())) return false;
    }
    return true;
}

void IGpuVertexData::wait() {
    visit_buffers([](IArrayBuffer& buffer){
        buffer.wait();
        return true;
    });
}

bool IGpuVertexData::copy_in_progress() {
    return !visit_buffers([](IArrayBuffer& buffer){
        return !buffer.copy_in_progress();
    });
}
