
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

bool DynamicContinuousTextureLayer::is_initialized() const {
    return true;
}

std::shared_ptr<IArrayBuffer> DynamicContinuousTextureLayer::fork() {
    throw std::runtime_error("DynamicContinuousTextureLayer::fork not available (only supported for static buffers)");
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

void DynamicContinuousTextureLayer::init_type_erased(
    const std::byte* begin,
    const std::byte* end)
{
    throw std::runtime_error("DynamicContinuousTextureLayer::set_type_erased called, but \"is_awaited=true\"");
    // This code was before the introduction of the "is_awaited" function.
    // if (begin != end) {
    //     throw std::runtime_error("DynamicContinuousTextureLayer::set_type_erased: only empty ranges are supported");
    // }
    // data_.bind();
}

void DynamicContinuousTextureLayer::substitute_type_erased(
    const std::byte* begin,
    const std::byte* end)
{
    throw std::runtime_error("DynamicContinuousTextureLayer::substitute_type_erased called, but \"is_awaited=true\"");
}
