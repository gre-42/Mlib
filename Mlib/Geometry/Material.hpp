#pragma once
#include <Mlib/Geometry/Material/Blend_Mode.hpp>
#include <Mlib/Geometry/Material/Occluded_Type.hpp>
#include <Mlib/Geometry/Material/Occluder_Type.hpp>
#include <Mlib/Geometry/Material/Wrap_Mode.hpp>
#include <Mlib/Geometry/Texture_Descriptor.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <Mlib/Scene_Graph/Aggregate_Mode.hpp>
#include <Mlib/Scene_Graph/Transformation_Mode.hpp>

namespace Mlib {

struct Material {
    TextureDescriptor texture_descriptor;
    std::string dirt_texture;
    OccludedType occluded_type = OccludedType::OFF;
    OccluderType occluder_type = OccluderType::OFF;
    bool occluded_by_black = true;
    BlendMode blend_mode = BlendMode::OFF;
    WrapMode wrap_mode_s = WrapMode::REPEAT;
    WrapMode wrap_mode_t = WrapMode::REPEAT;
    bool collide = true;
    AggregateMode aggregate_mode = AggregateMode::OFF;
    TransformationMode transformation_mode = TransformationMode::ALL;
    bool is_small = false;
    bool cull_faces = true;
    OrderableFixedArray<float, 3> ambience{0.5f, 0.5f, 0.5f};
    OrderableFixedArray<float, 3> diffusivity{1.f, 1.f, 1.f};
    OrderableFixedArray<float, 3> specularity{1.f, 1.f, 1.f};
    float draw_distance_add = 1000;
    float draw_distance_remove = 1010;
    size_t draw_distance_noperations = 0;
    inline Material& compute_color_mode() {
        texture_descriptor.color_mode = (blend_mode == BlendMode::OFF)
            ? ColorMode::RGB
            : ColorMode::RGBA;
        return *this;
    }
    std::strong_ordering operator <=> (const Material&) const = default;
};

}
