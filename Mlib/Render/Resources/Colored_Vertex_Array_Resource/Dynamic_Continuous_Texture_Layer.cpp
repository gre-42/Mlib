#include "Dynamic_Continuous_Texture_Layer.hpp"
#include <Mlib/Array/Fixed_Array.hpp>

using namespace Mlib;

DynamicContinuousTextureLayer::DynamicContinuousTextureLayer(size_t max_num_triangles)
    : data_{ max_num_triangles }
{}

DynamicContinuousTextureLayer::~DynamicContinuousTextureLayer() = default;

bool DynamicContinuousTextureLayer::copy_in_progress() const {
    return false;
}

void DynamicContinuousTextureLayer::wait() const {
    // Do nothing
}

void DynamicContinuousTextureLayer::update() {
    data_.update();
}

void DynamicContinuousTextureLayer::bind() const {
    data_.bind();
}

bool DynamicContinuousTextureLayer::is_awaited() const {
    return true;
}

std::shared_ptr<IArrayBuffer> DynamicContinuousTextureLayer::fork() {
    THROW_OR_ABORT("DynamicContinuousTextureLayer::fork not available (only supported for static buffers)");
}

void DynamicContinuousTextureLayer::append(const FixedArray<float, 3>& layers) {
    data_.append(layers);
}

FixedArray<float, 3>& DynamicContinuousTextureLayer::operator [] (size_t i) {
    return data_[i];
}

void DynamicContinuousTextureLayer::remove(size_t i) {
    data_.remove(i);
}

void DynamicContinuousTextureLayer::set_type_erased(
    const char* begin,
    const char* end,
    TaskLocation task_location)
{
    THROW_OR_ABORT("DynamicContinuousTextureLayer::set_type_erased called, but \"is_awaited=true\"");
    // This code was before the introduction of the "is_awaited" function.
    // if (begin != end) {
    //     THROW_OR_ABORT("DynamicContinuousTextureLayer::set_type_erased: only empty ranges are supported");
    // }
    // data_.bind();
}
