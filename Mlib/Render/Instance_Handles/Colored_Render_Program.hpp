#pragma once
#include <Mlib/Geometry/Material/Blend_Mode.hpp>
#include <Mlib/Geometry/Material/Color_Mode.hpp>
#include <Mlib/Geometry/Material/Interior_Textures.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <Mlib/Render/Instance_Handles/Render_Program.hpp>
#include <Mlib/Scene_Graph/Render_Pass.hpp>
#include <map>

namespace Mlib {

struct ColoredRenderProgram: public RenderProgram {
    GLint mvp_location;
    std::map<size_t, GLint> mvp_light_locations;
    GLint mvp_dirtmap_location;
    GLint r_location;
    std::map<size_t, GLint> light_dir_locations;
    // GLint light_pos;
    GLint view_dir;
    GLint view_pos;
    GLint horizontal_detailmap_remainder;
    std::map<size_t, GLint> pose_positions;
    std::map<size_t, GLint> pose_quaternions;
    std::map<size_t, GLint> light_ambiences;
    std::map<size_t, GLint> light_diffusivities;
    std::map<size_t, GLint> light_specularities;
    std::map<size_t, GLint> texture_color_locations;
    std::map<size_t, GLint> texture_alpha_locations;
    std::map<size_t, GLint> texture_lightmap_color_locations;
    std::map<size_t, GLint> texture_lightmap_depth_locations;
    std::map<size_t, GLint> texture_normalmap_locations;
    GLint texture_reflection_location;
    GLint texture_dirtmap_location;
    FixedArray<GLint, INTERIOR_COUNT> texture_interiormap_location;
    GLint texture_specularmap_location;
    GLint texture_dirt_location;
    GLint uv_offset_u_location;
    GLint vertex_scale_location;
    GLint uv_scale_location;
    GLint uv_offset_location;
    GLint alpha_distances_location;
};

struct RenderProgramIdentifier {
    ExternalRenderPassType render_pass;
    size_t nlights;
    size_t nbones;
    BlendMode blend_mode;
    OrderableFixedArray<float, 4> alpha_distances;
    size_t ntextures_color;
    size_t ntextures_alpha;
    size_t ntextures_normal;
    std::vector<size_t> lightmap_indices_color;
    std::vector<size_t> lightmap_indices_depth;
    bool has_specularmap;
    float reflection_strength;
    bool reflect_only_y;
    size_t ntextures_reflection;
    size_t ntextures_dirt;
    size_t ntextures_interior;
    OrderableFixedArray<float, 2> facade_edge_size;
    OrderableFixedArray<float, 2> facade_inner_size;
    OrderableFixedArray<float, 3> interior_size;
    bool has_horizontal_detailmap;
    ColorMode dirt_color_mode;
    bool has_instances;
    bool has_lookat;
    bool has_yangle;
    bool has_uv_offset_u;
    bool has_texture_layer;
    uint32_t nbillboard_ids;
    bool reorient_normals;
    bool reorient_uv0;
    OrderableFixedArray<float, 3> emissivity;
    OrderableFixedArray<float, 3> ambience;
    OrderableFixedArray<float, 3> diffusivity;
    OrderableFixedArray<float, 3> specularity;
    float specular_exponent;
    float alpha;
    bool orthographic;
    bool fragments_depend_on_distance;
    bool fragments_depend_on_normal;
    float dirtmap_offset;
    float dirtmap_discreteness;
    float dirt_scale;
    std::partial_ordering operator <=> (const RenderProgramIdentifier&) const = default;
};

}
