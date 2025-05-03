#include "Bvh_File_Resource.hpp"
#include <Mlib/Geometry/Mesh/Load/Load_Bvh.hpp>

using namespace Mlib;

BvhFileResource::BvhFileResource(
    const std::string& filename,
    const BvhConfig& config)
: bvh_loader{ std::make_unique<BvhLoader>(filename, config)}
{}

BvhFileResource::~BvhFileResource()
{}

void BvhFileResource::preload(const RenderableResourceFilter& filter) const {
    // Do nothing
}

StringWithHashUnorderedMap<OffsetAndQuaternion<float, float>> BvhFileResource::get_relative_poses(float time) const {
    return bvh_loader->get_relative_interpolated_frame(time);
}

StringWithHashUnorderedMap<OffsetAndQuaternion<float, float>> BvhFileResource::get_absolute_poses(float time) const {
    return bvh_loader->get_absolute_interpolated_frame(time);
}

float BvhFileResource::get_animation_duration() const {
    return bvh_loader->duration();
}
