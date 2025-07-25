#pragma once
#include <Mlib/Billboard_Id.hpp>
#include <Mlib/Geometry/Material/Blend_Mode.hpp>
#include <Mlib/Geometry/Material/Color_Mode.hpp>
#include <Mlib/Geometry/Material/Fresnel.hpp>
#include <Mlib/Geometry/Material/Interior_Textures.hpp>
#include <Mlib/Geometry/Material/Render_Pass.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <Mlib/Render/Instance_Handles/Render_Program.hpp>
#include <Mlib/Render/Instance_Handles/Texture_Layer_Properties.hpp>
#include <map>

namespace Mlib {

struct ColoredRenderProgram: public RenderProgram {
    GLint mvp_location;
    std::map<size_t, GLint> mvp_light_locations;
    std::map<size_t, GLint> mvp_skidmarks_locations;
    GLint mvp_dirtmap_location;
    GLint r_location;
    std::map<size_t, GLint> light_dir_locations;
    // GLint light_pos;
    GLint view_dir;
    GLint view_pos;
    GLint horizontal_detailmap_remainder;
    std::map<size_t, GLint> pose_positions;
    std::map<size_t, GLint> pose_quaternions;
    std::map<size_t, GLint> light_ambients;
    std::map<size_t, GLint> light_diffuses;
    std::map<size_t, GLint> light_speculars;
    std::map<size_t, GLint> texture_color_locations;
    std::map<size_t, GLint> texture_alpha_locations;
    std::map<size_t, GLint> texture_lightmap_color_locations;
    std::map<size_t, GLint> texture_lightmap_depth_locations;
    std::map<size_t, GLint> texture_normalmap_locations;
    std::map<size_t, GLint> texture_skidmark_locations;
    GLint texture_reflection_location;
    GLint texture_dirtmap_location;
    FixedArray<GLint, INTERIOR_COUNT_MAX> texture_interiormap_location = uninitialized;
    GLint texture_specularmap_location;
    GLint texture_dirt_location;
    GLint uv_offset_u_location;
    GLint vertex_scale_location;
    GLint uv_scale_location;
    GLint uv_offset_location;
    GLint texture_layers_location_atlas;
    GLint texture_layer_location_uniform;
    GLint alpha_distances_location;
    GLint dynamic_emissive_location;
};

struct RenderProgramIdentifier {
    size_t skidmarks_hash;
    size_t nbones;
    BlendMode blend_mode;
    OrderableFixedArray<float, 4> alpha_distances;
    OrderableFixedArray<float, 2> fog_distances;
    OrderableFixedArray<float, 3> fog_emissive;
    size_t ntextures_color;
    size_t ntextures_normal;
    size_t ntextures_alpha;
    bool has_dynamic_emissive;
    std::vector<size_t> lightmap_indices_color;
    std::vector<size_t> lightmap_indices_depth;
    bool has_specularmap;
    OrderableFixedArray<float, 3> reflectance;
    bool reflect_only_y;
    size_t ntextures_reflection;
    size_t ntextures_dirt;
    InteriorTextureSet interior_texture_set;
    OrderableFixedArray<float, 2> facade_inner_size;
    OrderableFixedArray<float, 3> interior_size;
    size_t nuv_indices;
    size_t ncweights;
    bool has_alpha;
    std::vector<float> continuous_layer_x;
    std::vector<float> continuous_layer_y;
    bool has_horizontal_detailmap;
    ColorMode dirt_color_mode;
    bool has_instances;
    bool has_lookat;
    bool has_yangle;
    bool has_rotation_quaternion;
    bool has_uv_offset_u;
    TextureLayerProperties texture_layer_properties;
    BillboardId nbillboard_ids;
    bool reorient_normals;
    bool reorient_uv0;
    OrderableFixedArray<float, 3> emissive;
    OrderableFixedArray<float, 3> ambient;
    OrderableFixedArray<float, 3> diffuse;
    OrderableFixedArray<float, 3> specular;
    float specular_exponent;
    OrderableFixedArray<float, 3> fresnel_emissive;
    FresnelReflectance fresnel;
    float alpha;
    bool orthographic;
    bool fragments_depend_on_distance;
    bool fragments_depend_on_normal;
    float dirtmap_offset;
    float dirtmap_discreteness;
    float dirt_scale;
    size_t texture_modifiers_hash;
    size_t lights_hash;
    std::partial_ordering operator <=> (const RenderProgramIdentifier&) const = default;
};

}
