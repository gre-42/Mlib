#include "Static_Billboard_Ids.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Mesh/Transformation_And_Billboard_Id.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Deallocate/Render_Try_Delete.hpp>

using namespace Mlib;

StaticBillboardIds::StaticBillboardIds(
    const std::vector<TransformationAndBillboardId>& instances,
    uint32_t num_billboard_atlas_components)
: instances_{instances},
  num_billboard_atlas_components_{num_billboard_atlas_components},
  buffer_{(GLuint)-1}
{}

StaticBillboardIds::~StaticBillboardIds() {
    if (buffer_ != (GLuint)-1) {
        try_delete_buffer(buffer_);
    }
}

void StaticBillboardIds::bind(GLuint attribute_index) const {
    std::vector<uint32_t> billboard_ids;
    billboard_ids.reserve(instances_.size());
    for (const TransformationAndBillboardId& m : instances_) {
        if (m.billboard_id >= num_billboard_atlas_components_) {
            THROW_OR_ABORT("Billboard ID too large");
        }
        billboard_ids.push_back(m.billboard_id);
    }
    if (buffer_ != (GLuint)-1) {
        THROW_OR_ABORT("Buffer has already been bound");
    } 
    CHK(glGenBuffers(1, &buffer_));
    if (buffer_ == (GLuint)-1) {
        THROW_OR_ABORT("Unsupported buffer index");
    }
    CHK(glBindBuffer(GL_ARRAY_BUFFER, buffer_));
    CHK(glBufferData(GL_ARRAY_BUFFER, integral_cast<GLsizeiptr>(sizeof(billboard_ids[0]) * billboard_ids.size()), billboard_ids.data(), GL_STATIC_DRAW));

    CHK(glEnableVertexAttribArray(attribute_index));
    CHK(glVertexAttribIPointer(attribute_index, 1, GL_UNSIGNED_INT, sizeof(billboard_ids[0]), nullptr));
    CHK(glVertexAttribDivisor(attribute_index, 1));
}
