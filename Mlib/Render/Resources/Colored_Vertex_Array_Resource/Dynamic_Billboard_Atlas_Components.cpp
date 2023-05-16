#include "Dynamic_Billboard_Atlas_Components.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Mesh/Transformation_And_Billboard_Id.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Deallocate/Render_Try_Delete.hpp>

using namespace Mlib;

DynamicBillboardAtlasComponents::DynamicBillboardAtlasComponents(
    uint32_t max_num_instances,
    uint32_t num_billboard_atlas_components)
: num_billboard_atlas_components_{num_billboard_atlas_components},
  max_num_instances_{max_num_instances},
  num_instances_{0}
{
    CHK(glGenBuffers(1, &buffer_));
    if (buffer_ == (GLuint)-1) {
        THROW_OR_ABORT("Unsupported buffer index");
    }
    CHK(glBindBuffer(GL_ARRAY_BUFFER, buffer_));
    CHK(glBufferData(GL_ARRAY_BUFFER, integral_cast<GLsizeiptr>(sizeof(uint32_t) * max_num_instances), nullptr, GL_DYNAMIC_DRAW));

    CHK(glBindBuffer(GL_ARRAY_BUFFER, buffer_));
    CHK(instances_ = (uint32_t*)glMapNamedBuffer(buffer_, GL_READ_WRITE));
    CHK(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

DynamicBillboardAtlasComponents::~DynamicBillboardAtlasComponents() {
    ABORT(glUnmapNamedBuffer(buffer_));
    ABORT(glDeleteBuffers(1, &buffer_));
}

void DynamicBillboardAtlasComponents::append(const TransformationAndBillboardId& m) {
    if (m.billboard_id >= num_billboard_atlas_components_) {
        THROW_OR_ABORT("Billboard ID too large");
    }
    if (num_instances_ == max_num_instances_) {
        return;
    }
    instances_[num_instances_++] = m.billboard_id;
}

void DynamicBillboardAtlasComponents::remove(uint32_t index) {
    if (index >= num_instances_) {
        THROW_OR_ABORT("Billboard index out of bounds");
    }
    if (num_instances_ > 0) {
        instances_[index] = instances_[--num_instances_];
    }
}

void DynamicBillboardAtlasComponents::bind(GLuint attribute_index) const {
    CHK(glBindBuffer(GL_ARRAY_BUFFER, buffer_));

    CHK(glEnableVertexAttribArray(attribute_index));
    CHK(glVertexAttribIPointer(attribute_index, 1, GL_UNSIGNED_INT, sizeof(uint32_t), nullptr));
    CHK(glVertexAttribDivisor(attribute_index, 1));
}
