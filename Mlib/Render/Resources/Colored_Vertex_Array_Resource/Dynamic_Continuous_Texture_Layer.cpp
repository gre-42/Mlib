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

void DynamicContinuousTextureLayer::append(const FixedArray<float, 3>& layers) {
    data_.append(layers);
}

FixedArray<float, 3>& DynamicContinuousTextureLayer::operator [] (size_t i) {
    return data_[i];
}

void DynamicContinuousTextureLayer::remove(size_t i) {
    data_.remove(i);
}

void DynamicContinuousTextureLayer::set_type_erased(const char* begin, const char* end) {
    if (begin != end) {
        THROW_OR_ABORT("DynamicContinuousTextureLayer::set_type_erased: only empty ranges are supported");
    }
    data_.bind();
}
