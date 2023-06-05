#include "Static_Position_YAngles.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Mesh/Transformation_And_Billboard_Id.hpp>
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Deallocate/Render_Deallocator.hpp>
#include <Mlib/Render/Deallocate/Render_Try_Delete.hpp>

using namespace Mlib;

StaticPositionYAngles::StaticPositionYAngles(const std::vector<TransformationAndBillboardId>& instances)
: instances_{instances},
  buffer_{(GLuint)-1},
  deallocation_token_{render_deallocator.insert([this](){deallocate();})}
{}

StaticPositionYAngles::~StaticPositionYAngles() {
    deallocate();
}

void StaticPositionYAngles::deallocate() {
    if (buffer_ != (GLuint)-1) {
        try_delete_buffer(buffer_);
    }
}

void StaticPositionYAngles::bind(GLuint attribute_index) const {
    std::vector<FixedArray<float, 4>> positions;
    positions.reserve(instances_.size());
    for (const TransformationAndBillboardId& m : instances_) {
        positions.emplace_back(
            m.transformation_matrix.t(0),
            m.transformation_matrix.t(1),
            m.transformation_matrix.t(2),
            std::atan2(-m.transformation_matrix.R(2, 0), m.transformation_matrix.R(0, 0)));
    }
    if (buffer_ != (GLuint)-1) {
        THROW_OR_ABORT("Buffer has already been bound");
    } 
    CHK(glGenBuffers(1, &buffer_));
    if (buffer_ == (GLuint)-1) {
        THROW_OR_ABORT("Unsupported buffer index");
    }
    CHK(glBindBuffer(GL_ARRAY_BUFFER, buffer_));
    CHK(glBufferData(GL_ARRAY_BUFFER, integral_cast<GLsizeiptr>(sizeof(positions[0]) * positions.size()), positions.data(), GL_STATIC_DRAW));

    CHK(glEnableVertexAttribArray(attribute_index));
    CHK(glVertexAttribPointer(attribute_index, 4, GL_FLOAT, GL_FALSE, sizeof(positions[0]), nullptr));
    CHK(glVertexAttribDivisor(attribute_index, 1));
}
