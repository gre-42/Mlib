#include "Dynamic_Triangle.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Colored_Vertex.hpp>

using namespace Mlib;

DynamicTriangle::DynamicTriangle(size_t max_num_triangles)
    : data_{ max_num_triangles }
{}

DynamicTriangle::~DynamicTriangle() = default;

bool DynamicTriangle::copy_in_progress() const {
    return false;
}

void DynamicTriangle::wait() const {
    // Do nothing
}

void DynamicTriangle::update() {
    data_.update();
}

void DynamicTriangle::bind() const {
    return data_.bind();
}

bool DynamicTriangle::is_awaited() const {
    return true;
}

std::shared_ptr<IArrayBuffer> DynamicTriangle::fork() {
    THROW_OR_ABORT("DynamicTriangle::fork not available (only supported for static buffers)");
}

void DynamicTriangle::append(const FixedArray<ColoredVertex<float>, 3>& t) {
    data_.append(t);
}

void DynamicTriangle::remove(size_t i) {
    data_.remove(i);
}

void DynamicTriangle::set_type_erased(
    const char* begin,
    const char* end,
    TaskLocation task_location)
{
    THROW_OR_ABORT("DynamicTriangle::set_type_erased called, but \"is_awaited=true\"");
    // This code was before the introduction of the "is_awaited" function.
    // if (begin != end) {
    //     THROW_OR_ABORT("DynamicTriangle::set_type_erased: only empty ranges are supported");
    // }
    // data_.bind();
}
