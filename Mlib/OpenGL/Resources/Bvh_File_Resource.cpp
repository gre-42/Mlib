#include "Bvh_File_Resource.hpp"
#include <Mlib/Geometry/Mesh/Load/Load_Bvh.hpp>

using namespace Mlib;

BvhFileResource::BvhFileResource(
    const std::string& filename,
    const BvhConfig& config)
    : ISceneNodeResource{"BvhFileResource"}
    , bvh_loader{ std::make_unique<BvhLoader>(filename, config)}
{}

BvhFileResource::~BvhFileResource()
{}

void BvhFileResource::preload(const RenderableResourceFilter& filter) {
    // Do nothing
}

UUVector<OffsetAndQuaternion<float, float>> BvhFileResource::get_relative_poses(
    float time,
    const StringWithHashUnorderedMap<uint32_t>& bone_indices) const
{
    return bvh_loader->get_relative_interpolated_frame(time, bone_indices);
}

UUVector<OffsetAndQuaternion<float, float>> BvhFileResource::get_absolute_poses(
    float time,
    const StringWithHashUnorderedMap<uint32_t>& bone_indices) const
{
    return bvh_loader->get_absolute_interpolated_frame(time, bone_indices);
}

float BvhFileResource::get_animation_duration() const {
    return bvh_loader->duration();
}
