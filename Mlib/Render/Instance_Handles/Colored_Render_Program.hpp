#pragma once
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <Mlib/Geometry/Material/Occluder_Type.hpp>
#include <Mlib/Geometry/Material/Blend_Mode.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <Mlib/Render/Instance_Handles/Render_Program.hpp>
#include <map>

namespace Mlib {

struct ColoredRenderProgram: public RenderProgram {
    GLint mvp_location;
    std::map<size_t, GLint> mvp_light_locations;
    GLint mvp_dirtmap_location;
    GLint m_location;
    std::map<size_t, GLint> light_dir_locations;
    // GLint light_pos;
    GLint view_dir;
    GLint view_pos;
    std::map<size_t, GLint> pose_positions;
    std::map<size_t, GLint> pose_quaternions;
    std::map<size_t, GLint> light_ambiences;
    std::map<size_t, GLint> light_diffusivities;
    std::map<size_t, GLint> light_specularities;
    GLint texture1_location;
    std::map<size_t, GLint> texture_lightmap_color_locations;
    std::map<size_t, GLint> texture_lightmap_depth_locations;
    GLint texture_normalmap_location;
    GLint texture_dirtmap_location;
    GLint texture_dirt_location;
};

struct RenderProgramIdentifier {
    OccluderType occluder_type;
    size_t nlights;
    BlendMode blend_mode;
    bool has_texture;
    bool has_lightmap_color;
    bool has_lightmap_depth;
    bool has_normalmap;
    bool has_dirtmap;
    bool has_instances;
    bool has_lookat;
    bool reorient_normals;
    bool calculate_lightmap;
    OrderableFixedArray<float, 3> ambience;
    OrderableFixedArray<float, 3> diffusivity;
    OrderableFixedArray<float, 3> specularity;
    bool orthographic;
    float dirtmap_discreteness;
    std::strong_ordering operator <=> (const RenderProgramIdentifier&) const = default;
};

}
