#include "Colored_Vertex_Array_Resource.hpp"
#include <Mlib/Env.hpp>
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Geometry/Material/Blend_Distances.hpp>
#include <Mlib/Geometry/Material/Blend_Map_Texture.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array_Filter.hpp>
#include <Mlib/Geometry/Mesh/Import_Bone_Weights.hpp>
#include <Mlib/Geometry/Mesh/Transformation_And_Billboard_Id.hpp>
#include <Mlib/Geometry/Mesh/Triangle_Rays.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Images/Coordinates_Fixed.hpp>
#include <Mlib/Images/Revert_Axis.hpp>
#include <Mlib/Images/Vectorial_Pixels.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Gen_Shader_Text.hpp>
#include <Mlib/Render/Instance_Handles/Colored_Render_Program.hpp>
#include <Mlib/Render/Renderables/Renderable_Colored_Vertex_Array.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Render/Resources/Substitution_Info.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Light.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Instantiation_Options.hpp>
#include <Mlib/Stats/Mean.hpp>
#include <Mlib/Strings/String.hpp>
#include <iostream>
#include <mutex>

using namespace Mlib;

static const size_t ANIMATION_NINTERPOLATED = 4;
struct ShaderBoneWeight {
    unsigned char indices[ANIMATION_NINTERPOLATED];
    float weights[ANIMATION_NINTERPOLATED];
};

struct ShaderInteriorMappedFacade {
    FixedArray<float, 3> bottom_left;
    FixedArray<float, 2> multiplier;
};

static const size_t IDX_POSITION = 0;
static const size_t IDX_COLOR = 1;
static const size_t IDX_UV = 2;
static const size_t IDX_NORMAL = 3;
static const size_t IDX_TANGENT = 4;
static const size_t IDX_INSTANCE_ATTRS = 5;
static const size_t IDX_BILLBOARD_IDS = 6;
static const size_t IDX_BONE_INDICES = 7;
static const size_t IDX_BONE_WEIGHTS = 8;
static const size_t IDX_INTERIOR_MAPPING_BOTTOM_LEFT = 9;
static const size_t IDX_INTERIOR_MAPPING_MULTIPLIER = 10;

static GenShaderText vertex_shader_text_gen{[](
    const std::vector<std::pair<TransformationMatrix<float, double, 3>, Light*>>& lights,
    const std::vector<BlendMapTexture*>& textures,
    size_t nlights,
    size_t ntextures_color,
    uint32_t nbillboard_ids,
    bool has_lightmap_color,
    bool has_lightmap_depth,
    bool has_normalmap,
    bool has_reflection_map,
    bool has_dirtmap,
    bool has_interiormap,
    bool has_diffusivity,
    bool has_specularity,
    bool has_instances,
    bool has_lookat,
    bool has_yangle,
    bool has_uv_offset_u,
    size_t nbones,
    bool reorient_normals,
    bool reorient_uv0,
    bool orthographic,
    bool fragments_depend_on_distance,
    bool fragments_depend_on_normal)
{
    assert_true(nlights == lights.size());
    std::stringstream sstr;
    sstr << "#version 330 core" << std::endl;
    sstr << "uniform mat4 MVP;" << std::endl;
    sstr << "layout (location=" << IDX_POSITION << ") in vec3 vPos;" << std::endl;
    sstr << "layout (location=" << IDX_COLOR << ") in vec3 vCol;" << std::endl;
    sstr << "layout (location=" << IDX_UV << ") in vec2 vTexCoord;" << std::endl;
    if (reorient_uv0 || has_diffusivity || has_specularity || has_normalmap || fragments_depend_on_normal) {
        sstr << "layout (location=" << IDX_NORMAL << ") in vec3 vNormal;" << std::endl;
    }
    if (has_normalmap || has_interiormap) {
        sstr << "layout (location=" << IDX_TANGENT << ") in vec3 vTangent;" << std::endl;
    }
    if (has_instances) {
        if (has_yangle) {
            sstr << "layout (location=" << IDX_INSTANCE_ATTRS << ") in vec4 instancePosition;" << std::endl;
        } else {
            sstr << "layout (location=" << IDX_INSTANCE_ATTRS << ") in vec3 instancePosition;" << std::endl;
        }
    } else if (has_lookat && !orthographic) {
        sstr << "const vec3 instancePosition = vec3(0, 0, 0);" << std::endl;
    }
    if (nbillboard_ids != 0) {
        sstr << "layout (location=" << IDX_BILLBOARD_IDS << ") in uint billboard_id;" << std::endl;
        sstr << "uniform vec2 vertex_scale[" << nbillboard_ids << "];" << std::endl;
        sstr << "uniform vec2 uv_scale[" << nbillboard_ids << "];" << std::endl;
        sstr << "uniform vec2 uv_offset[" << nbillboard_ids << "];" << std::endl;
        if (!orthographic) {
            sstr << "uniform vec4 alpha_distances[" << nbillboard_ids << "];" << std::endl;
            sstr << "out float alpha_fac_v;" << std::endl;
        }
    }
    if (nbones != 0) {
        sstr << "layout (location=" << IDX_BONE_INDICES << ") in lowp uvec" << ANIMATION_NINTERPOLATED << " bone_ids;" << std::endl;
        sstr << "layout (location=" << IDX_BONE_WEIGHTS << ") in vec" << ANIMATION_NINTERPOLATED << " bone_weights;" << std::endl;
        sstr << "uniform vec3 bone_positions[" << nbones << "];" << std::endl;
        sstr << "uniform vec4 bone_quaternions[" << nbones << "];" << std::endl;
    }
    if (has_uv_offset_u) {
        sstr << "uniform float uv_offset_u;" << std::endl;
    }
    sstr << "out vec3 color;" << std::endl;
    sstr << "out vec2 tex_coord;" << std::endl;
    if (has_lightmap_color || has_lightmap_depth) {
        if (lights.empty()) {
            throw std::runtime_error("No lights despite has_lightmap_color or has_lightmap_depth");
        }
        sstr << "uniform mat4 MVP_light[" << lights.size() << "];" << std::endl;
        // vec4 to avoid clipping problems
        sstr << "out vec4 FragPosLightSpace[" << lights.size() << "];" << std::endl;
    }
    if (has_normalmap || has_interiormap) {
        sstr << "out vec3 tangent;" << std::endl;
        sstr << "out vec3 bitangent;" << std::endl;
    }
    if (has_dirtmap) {
        sstr << "uniform mat4 MVP_dirtmap;" << std::endl;
        sstr << "out vec2 tex_coord_dirtmap;" << std::endl;
    }
    if (has_interiormap) {
        sstr << "layout (location=" << IDX_INTERIOR_MAPPING_BOTTOM_LEFT << ") in vec3 interior_bottom_left;" << std::endl;
        sstr << "layout (location=" << IDX_INTERIOR_MAPPING_MULTIPLIER << ") in vec2 interior_multiplier;" << std::endl;
        sstr << "out vec3 interior_bottom_left_fs;" << std::endl;
        sstr << "out vec2 interior_multiplier_fs;" << std::endl;
    }
    if (reorient_uv0 || reorient_normals || has_specularity || (fragments_depend_on_distance && !orthographic) || has_interiormap || has_reflection_map) {
        sstr << "out vec3 FragPos;" << std::endl;
    }
    if (reorient_uv0 || has_diffusivity || has_specularity || fragments_depend_on_normal) {
        sstr << "out vec3 Normal;" << std::endl;
    }
    if (has_lookat) {
        if (orthographic) {
            sstr << "uniform vec3 viewDir;" << std::endl;
        } else {
            sstr << "uniform vec3 viewPos;" << std::endl;
        }
    }
    sstr << "void main()" << std::endl;
    sstr << "{" << std::endl;
    if (has_interiormap) {
        sstr << "    interior_bottom_left_fs = interior_bottom_left;" << std::endl;
        sstr << "    interior_multiplier_fs = interior_multiplier;" << std::endl;
    }
    sstr << "    vec3 vPosInstance;" << std::endl;
    if (reorient_uv0 || has_diffusivity || has_specularity || fragments_depend_on_normal) {
        sstr << "    vec3 vNormalInstance;" << std::endl;
    }
    if (nbones != 0) {
        sstr << "    vPosInstance = vec3(0, 0, 0);" << std::endl;
        if (reorient_uv0 || has_diffusivity || has_specularity || fragments_depend_on_normal) {
            sstr << "    vNormalInstance = vNormal;" << std::endl;
        }
        for (size_t k = 0; k < ANIMATION_NINTERPOLATED; ++k) {
            static std::map<unsigned char, char> m{
                {(unsigned char)0, 'x'},
                {(unsigned char)1, 'y'},
                {(unsigned char)2, 'z'},
                {(unsigned char)3, 'w'}};
            sstr << "    {" << std::endl;
            sstr << "        lowp uint i = bone_ids." << m.at((unsigned char)k) << ";" << std::endl;
            sstr << "        float weight = bone_weights." << m.at((unsigned char)k) << ";" << std::endl;
            sstr << "        vec3 o = bone_positions[i];" << std::endl;
            sstr << "        vec4 v = bone_quaternions[i];" << std::endl;
            sstr << "        vec3 p = vPos;" << std::endl;
            sstr << "        vPosInstance += weight * (o + p + 2 * cross(v.xyz, cross(v.xyz, p) + v.w * p));" << std::endl;
            sstr << "    }" << std::endl;
        }
    } else {
        sstr << "    vPosInstance = vPos;" << std::endl;
        if (reorient_uv0 || has_diffusivity || has_specularity || fragments_depend_on_normal) {
            sstr << "    vNormalInstance = vNormal;" << std::endl;
        }
    }
    if (nbillboard_ids != 0) {
        sstr << "    vPosInstance.xy = vPosInstance.xy * vertex_scale[billboard_id];" << std::endl;
        sstr << "    tex_coord = vTexCoord * uv_scale[billboard_id] + uv_offset[billboard_id];" << std::endl;
    } else {
        sstr << "    tex_coord = vTexCoord;" << std::endl;
    }
    // if (has_lookat && !has_instances) {
    //     throw std::runtime_error("has_lookat requires has_instances");
    // }
    if (has_yangle && !has_instances) {
        throw std::runtime_error("has_yangle requires has_instances");
    }
    if (has_lookat || has_yangle) {
        if (has_yangle) {
            sstr << "    vec2 dz_xz = vec2(sin(instancePosition.w), cos(instancePosition.w));" << std::endl;
        } else if (orthographic) {
            sstr << "    vec2 dz_xz = normalize(viewDir.xz);" << std::endl;
        } else {
            sstr << "    vec2 dz_xz = normalize(viewPos.xz - instancePosition.xz);" << std::endl;
        }
        sstr << "    vec3 dz = vec3(dz_xz.x, 0, dz_xz.y);" << std::endl;
        sstr << "    vec3 dy = vec3(0, 1, 0);" << std::endl;
        sstr << "    vec3 dx = normalize(cross(dy, dz));" << std::endl;
        sstr << "    mat3 lookat;" << std::endl;
        sstr << "    lookat[0] = dx;" << std::endl;
        sstr << "    lookat[1] = dy;" << std::endl;
        sstr << "    lookat[2] = dz;" << std::endl;
        sstr << "    vPosInstance = lookat * vPosInstance;" << std::endl;
        if (has_instances) {
            sstr << "    vPosInstance += instancePosition.xyz;" << std::endl;
        }
        if (reorient_uv0 || has_diffusivity || has_specularity || fragments_depend_on_normal) {
            sstr << "    vNormalInstance = lookat * vNormalInstance;" << std::endl;
        }
    } else if (has_instances && !has_lookat) {
        sstr << "    vPosInstance = vPosInstance + instancePosition;" << std::endl;
    }
    sstr << "    gl_Position = MVP * vec4(vPosInstance, 1.0);" << std::endl;
    sstr << "    color = vCol;" << std::endl;
    if ((nbillboard_ids != 0) && !orthographic) {
        sstr << "    {" << std::endl;
        sstr << "        float dist = distance(viewPos, vPosInstance);" << std::endl;
        sstr << "        vec4 ads = alpha_distances[billboard_id];" << std::endl;
        sstr << "        if ((dist < ads[0]) || (dist > ads[3])) {" << std::endl;
        sstr << "            alpha_fac_v = 0;" << std::endl;
        sstr << "        } else if (dist < ads[1]) {" << std::endl;
        sstr << "            alpha_fac_v = (dist - ads[0]) / (ads[1] - ads[0]);" << std::endl;
        sstr << "        } else if (dist > ads[2]) {" << std::endl;
        sstr << "            alpha_fac_v = (ads[3] - dist) / (ads[3] - ads[2]);" << std::endl;
        sstr << "        } else {" << std::endl;
        sstr << "            alpha_fac_v = 1;" << std::endl;
        sstr << "        }" << std::endl;
        sstr << "    }" << std::endl;
    }
    if (has_uv_offset_u) {
        sstr << "    tex_coord.s += uv_offset_u;" << std::endl;
    }
    if (has_lightmap_color || has_lightmap_depth) {
        sstr << "    for (int i = 0; i < " << lights.size() << "; ++i) {" << std::endl;
        sstr << "        FragPosLightSpace[i] = MVP_light[i] * vec4(vPosInstance, 1.0);" << std::endl;
        sstr << "    }" << std::endl;
    }
    if (has_dirtmap) {
        sstr << "    vec4 pos4_dirtmap = MVP_dirtmap * vec4(vPosInstance, 1.0);" << std::endl;
        sstr << "    tex_coord_dirtmap = (pos4_dirtmap.xy / pos4_dirtmap.w + 1) / 2;" << std::endl;
    }
    if (reorient_uv0 || reorient_normals || has_specularity || (fragments_depend_on_distance && !orthographic) || has_interiormap || has_reflection_map) {
        sstr << "    FragPos = vPosInstance;" << std::endl;
    }
    if (reorient_uv0 || has_diffusivity || has_specularity || fragments_depend_on_normal) {
        sstr << "    Normal = vNormalInstance;" << std::endl;
    }
    if (has_normalmap || has_interiormap) {
        sstr << "    tangent = vTangent;" << std::endl;
        sstr << "    bitangent = cross(Normal, tangent);" << std::endl;
    }
    sstr << "}" << std::endl;
    if (getenv_default_bool("PRINT_SHADERS", false)) {
        std::cerr << std::endl;
        std::cerr << std::endl;
        std::cerr << std::endl;
        std::cerr << "Vertex" << std::endl;
        if (!textures.empty()) {
            std::cerr << "Color: " + textures[0]->texture_descriptor.color << std::endl;
        }
        std::cerr << sstr.str() << std::endl;
    }
    return sstr.str();
}};

static GenShaderText fragment_shader_text_textured_rgb_gen{[](
    const std::vector<std::pair<TransformationMatrix<float, double, 3>, Light*>>& lights,
    const std::vector<BlendMapTexture*>& textures,
    const std::vector<size_t>& light_noshadow_indices,
    const std::vector<size_t>& light_shadow_indices,
    const std::vector<size_t>& black_shadow_indices,
    size_t nlights,
    size_t ntextures_color,
    size_t ntextures_normal,
    bool has_lightmap_color,
    bool has_lightmap_depth,
    bool has_specularmap,
    bool has_normalmap,
    size_t nbillboard_ids,
    float reflection_strength,
    bool reflect_only_y,
    bool has_dirtmap,
    bool has_interiormap,
    const OrderableFixedArray<float, 2>& facade_edge_size,
    const OrderableFixedArray<float, 2>& facade_inner_size,
    const OrderableFixedArray<float, 3>& interior_size,
    ColorMode dirt_color_mode,
    const OrderableFixedArray<float, 3>& emissivity,
    const OrderableFixedArray<float, 3>& ambience,
    const OrderableFixedArray<float, 3>& diffusivity,
    const OrderableFixedArray<float, 3>& specularity,
    float alpha,
    float alpha_threshold,
    const OrderableFixedArray<float, 4>& alpha_distances,
    ExternalRenderPassType render_pass,
    bool reorient_normals,
    bool reorient_uv0,
    bool orthographic,
    bool fragments_depend_on_distance,
    bool fragments_depend_on_normal,
    float dirtmap_offset,
    float dirtmap_discreteness,
    float dirt_scale)
{
    assert_true(nlights == lights.size());
    std::stringstream sstr;
    sstr << "#version 330 core" << std::endl;
    sstr << "in vec3 color;" << std::endl;
    if (ntextures_color != 0) {
        sstr << "in vec2 tex_coord;" << std::endl;
    }
    sstr << "out vec4 frag_color;" << std::endl;
    if ((nbillboard_ids != 0) && !orthographic) {
        sstr << "in float alpha_fac_v;" << std::endl;
    }
    if (reflection_strength != 0.f) {
        sstr << "uniform mat3 R;" << std::endl;
    }
    if (ntextures_color != 0) {
        sstr << "uniform sampler2D textures_color[" << ntextures_color << "];" << std::endl;
    }
    if (has_lightmap_color || has_lightmap_depth) {
        if (lights.empty()) {
            throw std::runtime_error("No lights despite has_lightmap_color or has_lightmap_depth");
        }
        sstr << "in vec4 FragPosLightSpace[" << lights.size() << "];" << std::endl;
    }
    assert_true(!(has_lightmap_color && has_lightmap_depth));
    if (has_lightmap_color) {
        sstr << "uniform sampler2D texture_light_color[" << lights.size() << "];" << std::endl;
    }
    if (has_lightmap_depth) {
        sstr << "uniform sampler2D texture_light_depth[" << lights.size() << "];" << std::endl;
    }
    if (has_normalmap || has_interiormap) {
        sstr << "in vec3 tangent;" << std::endl;
        sstr << "in vec3 bitangent;" << std::endl;
    }
    if (has_normalmap) {
        sstr << "uniform sampler2D texture_normalmap[" << ntextures_normal << "];" << std::endl;
    }
    if (reflection_strength != 0.f) {
        sstr << "uniform samplerCube texture_reflection;" << std::endl;
    }
    if (has_dirtmap) {
        sstr << "in vec2 tex_coord_dirtmap;" << std::endl;
        sstr << "uniform sampler2D texture_dirtmap;" << std::endl;
        sstr << "uniform sampler2D texture_dirt;" << std::endl;
    }
    if (has_interiormap) {
        sstr << "in vec3 interior_bottom_left_fs;" << std::endl;
        sstr << "in vec2 interior_multiplier_fs;" << std::endl;
        sstr << "uniform sampler2D texture_interior[" << INTERIOR_COUNT << "];" << std::endl;
    }
    if (has_specularmap) {
        sstr << "uniform sampler2D texture_specularmap;" << std::endl;
    }
    if (!diffusivity.all_equal(0) || !specularity.all_equal(0) || fragments_depend_on_normal) {
        sstr << "in vec3 Normal;" << std::endl;

        // sstr << "uniform vec3 lightPos;" << std::endl;
        sstr << "uniform vec3 lightDir[" << lights.size() << "];" << std::endl;
    }
    if (!ambience.all_equal(0)) {
        sstr << "uniform vec3 lightAmbience[" << lights.size() << "];" << std::endl;
    }
    if (!diffusivity.all_equal(0)) {
        sstr << "uniform vec3 lightDiffusivity[" << lights.size() << "];" << std::endl;
    }
    if (!specularity.all_equal(0)) {
        sstr << "uniform vec3 lightSpecularity[" << lights.size() << "];" << std::endl;
    }
    {
        bool pred0 = reorient_uv0 || reorient_normals || !specularity.all_equal(0) || (fragments_depend_on_distance && !orthographic) || (reflection_strength != 0.f);
        if (pred0 || has_interiormap || (reflection_strength != 0.f)) {
            sstr << "in vec3 FragPos;" << std::endl;
            if (pred0 && orthographic) {
                sstr << "uniform vec3 viewDir;" << std::endl;
            }
            if ((pred0 && !orthographic) || has_interiormap || (reflection_strength != 0.f)) {
                sstr << "uniform vec3 viewPos;" << std::endl;
            }
        }
    }
    if (!lights.empty()) {
        if (!ambience.all_equal(0)) {
            sstr << "vec3 phong_ambient(in int i) {" << std::endl;
            sstr << "    vec3 fragAmbience = vec3(" << ambience(0) << ", " << ambience(1) << ", " << ambience(2) << ");" << std::endl;
            sstr << "    return fragAmbience * lightAmbience[i];" << std::endl;
            sstr << "}" << std::endl;
        }
        if (!diffusivity.all_equal(0)) {
            sstr << "vec3 phong_diffuse(in int i, in vec3 norm) {" << std::endl;
            sstr << "    vec3 fragDiffusivity = vec3(" << diffusivity(0) << ", " << diffusivity(1) << ", " << diffusivity(2) << ");" << std::endl;
            sstr << "    float diff = max(dot(norm, lightDir[i]), 0.0);" << std::endl;
            sstr << "    return fragDiffusivity * diff * lightDiffusivity[i];" << std::endl;
            sstr << "}" << std::endl;
        }
        if (!specularity.all_equal(0)) {
            sstr << "vec3 phong_specular(in int i, in vec3 norm) {" << std::endl;
            sstr << "    vec3 fragSpecularity = vec3(" << specularity(0) << ", " << specularity(1) << ", " << specularity(2) << ");" << std::endl;
            if (!orthographic) {
                sstr << "    vec3 viewDir = normalize(viewPos - FragPos);" << std::endl;
            }
            sstr << "    vec3 reflectDir = reflect(-lightDir[i], norm);  " << std::endl;
            sstr << "    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 4);" << std::endl;
            sstr << "    return fragSpecularity * spec * lightSpecularity[i];" << std::endl;
            sstr << "}" << std::endl;
        }
    }
    if (has_interiormap) {
        sstr << "bool is_in_interior(mat3 TBN) {" << std::endl;
        sstr << "    vec3 rel_view_pos = transpose(TBN) * (viewPos - interior_bottom_left_fs);" << std::endl;
        sstr << "    vec2 rel_frag_pos = (transpose(TBN) * (FragPos - interior_bottom_left_fs)).xy;" << std::endl;
        sstr << "    rel_view_pos.xy *= interior_multiplier_fs;" << std::endl;
        sstr << "    rel_frag_pos *= interior_multiplier_fs;" << std::endl;
        sstr << "    vec3 rel_view_dir = vec3(rel_frag_pos, 0) - rel_view_pos;" << std::endl;
        sstr << "    float best_alpha = 1. / 0.;" << std::endl;
        sstr << "    int best_axis;" << std::endl;
        sstr << "    bool best_sign;" << std::endl;
        sstr << "    vec2 best_uv;" << std::endl;
        sstr << "    vec2 facade_edge_size = vec2(" <<
            facade_edge_size(0) << ", " <<
            facade_edge_size(1) << ");" << std::endl;
        sstr << "    vec2 facade_inner_size = vec2(" <<
            facade_inner_size(0) << ", " <<
            facade_inner_size(1) << ");" << std::endl;
        sstr << "    vec3 interior_size = vec3(" <<
            interior_size(0) << ", " <<
            interior_size(1) << ", " <<
            interior_size(2) << ");" << std::endl;
        sstr << "    vec2 w = interior_size.xy + facade_inner_size;" << std::endl;
        sstr << "    vec2 bottom = floor((rel_frag_pos - facade_edge_size) / w) * w + facade_edge_size;" << std::endl;
        sstr << "    if (any(lessThan(rel_frag_pos, bottom))) {" << std::endl;
        sstr << "        return false;" << std::endl;
        sstr << "    };" << std::endl;
        sstr << "    if (any(greaterThan(rel_frag_pos, bottom + interior_size.xy))) {" << std::endl;
        sstr << "        return false;" << std::endl;
        sstr << "    };" << std::endl;
        for (size_t axis = 0; axis < 2; ++axis) {
            char axis0;
            char axis1;
            if (axis == 0) { axis0 = 'z'; axis1 = 'y'; }
            if (axis == 1) { axis0 = 'x'; axis1 = 'z'; }
            sstr << "    {" << std::endl;
            sstr << "        if (rel_view_dir[" << axis << "] < 0) {" << std::endl;
            sstr << "            float alpha = (bottom[" << axis << "] - rel_view_pos[" << axis << "]) / rel_view_dir[" << axis << "];" << std::endl;
            sstr << "            if (alpha < best_alpha) {" << std::endl;
            sstr << "                best_alpha = alpha;" << std::endl;
            sstr << "                best_axis = " << axis << ";" << std::endl;
            sstr << "                best_sign = false;" << std::endl;
            sstr << "                best_uv = ((rel_view_pos + alpha * rel_view_dir - vec3(bottom, 0)) / interior_size)." << axis0 << axis1 << ";" << std::endl;
            sstr << "                best_uv[" << axis << "] = -best_uv[" << axis << "];" << std::endl;
            sstr << "            }" << std::endl;
            sstr << "        } else {" << std::endl;
            sstr << "            float alpha = (interior_size[" << axis << "] + bottom[" << axis << "] - rel_view_pos[" << axis << "]) / rel_view_dir[" << axis << "];" << std::endl;
            sstr << "            if (alpha < best_alpha) {" << std::endl;
            sstr << "                best_alpha = alpha;" << std::endl;
            sstr << "                best_axis = " << axis << ";" << std::endl;
            sstr << "                best_sign = true;" << std::endl;
            sstr << "                best_uv = ((rel_view_pos + alpha * rel_view_dir - vec3(bottom, 0)) / interior_size)." << axis0 << axis1 << ";" << std::endl;
            sstr << "                best_uv[" << axis << "] = 1 + best_uv[" << axis << "];" << std::endl;
            sstr << "            }" << std::endl;
            sstr << "        }" << std::endl;
            sstr << "    }" << std::endl;
        }
        sstr << "    float alpha = (-interior_size[2] - rel_view_pos[2]) / rel_view_dir[2];" << std::endl;
        sstr << "    if (alpha < best_alpha) {" << std::endl;
        sstr << "        best_alpha = alpha;" << std::endl;
        sstr << "        best_axis = 2;" << std::endl;
        sstr << "        best_sign = false;" << std::endl;
        sstr << "        best_uv = ((rel_view_pos + alpha * rel_view_dir - vec3(bottom, 0)) / interior_size).xy;" << std::endl;
        sstr << "    }" << std::endl;
        sstr << "    frag_color = texture(texture_interior[2 * best_axis + int(best_sign)], best_uv);" << std::endl;
        sstr << "    return true;" << std::endl;
        sstr << "}" << std::endl;
    }
    sstr << "void main()" << std::endl;
    sstr << "{" << std::endl;
    if ((nbillboard_ids != 0) && !orthographic) {
        sstr << "    float alpha_fac = alpha_fac_v;" << std::endl;
    } else {
        sstr << "    float alpha_fac = 1;" << std::endl;
    }
    if (alpha_distances != default_linear_distances) {
        if (orthographic) {
            // throw std::runtime_error("Orthographic not supported with alpha distances");
            sstr << "    float dist = 1. / 0.;" << std::endl;
        } else {
            sstr << "    float dist = distance(FragPos, viewPos);" << std::endl;
        }
    }
    if (alpha_distances(0) != 0) {
        sstr << "    if (dist < " << alpha_distances(0) << ')' << std::endl;
        sstr << "        discard;" << std::endl;
    }
    if (alpha_distances(3) != INFINITY) {
        sstr << "    if (dist > " << alpha_distances(3) << ')' << std::endl;
        sstr << "        discard;" << std::endl;
    }
    if (alpha_distances(0) != alpha_distances(1)) {
        sstr << "    if (dist < " << alpha_distances(1) << ") {" << std::endl;
        sstr << "        alpha_fac = (dist - " << alpha_distances(0) << ") / " << (alpha_distances(1) - alpha_distances(0)) << ";" << std::endl;
        sstr << "    }" << std::endl;
    }
    if (alpha_distances(3) != alpha_distances(2)) {
        sstr << "    if (dist > " << alpha_distances(2) << ") {" << std::endl;
        sstr << "        alpha_fac = (" << alpha_distances(3) << " - dist) / " << (alpha_distances(3) - alpha_distances(2)) << ";" << std::endl;
        sstr << "    }" << std::endl;
    }
    sstr << "    alpha_fac *= " << alpha << ';' << std::endl;
    auto compute_normal = [&](){
        if (!diffusivity.all_equal(0) || !specularity.all_equal(0) || fragments_depend_on_normal) {
            // sstr << "    vec3 norm = normalize(Normal);" << std::endl;
            sstr << "    vec3 norm = normalize(Normal);" << std::endl;
            // sstr << "    vec3 lightDir = normalize(lightPos - FragPos);" << std::endl;
        }
        if (reorient_uv0 || reorient_normals) {
            if (orthographic) {
                sstr << "    if (dot(norm, viewDir) < 0) {" << std::endl;
            } else {
                // From: https://stackoverflow.com/questions/2523439/ipad-glsl-from-within-a-fragment-shader-how-do-i-get-the-surface-not-vertex
                sstr << "    vec3 normalvector = cross(dFdx(FragPos), dFdy(FragPos));" << std::endl;
                sstr << "    if (dot(norm, normalvector) * dot(normalvector, viewPos - FragPos) < 0) {" << std::endl;
            }
            if (reorient_normals) {
                sstr << "        norm = -norm;" << std::endl;
            }
            if (reorient_uv0) {
                sstr << "        tex_coord_flipped.s = -tex_coord_flipped.s;" << std::endl;
            }
            sstr << "    }" << std::endl;
        }
    };
    if (ntextures_color != 0) {
        sstr << "    vec2 tex_coord_flipped = tex_coord;" << std::endl;
    }
    if (ntextures_color == 1) {
        if (reorient_uv0) {
            compute_normal();
        }
        sstr << "    vec4 texture_color_ambient_diffuse = texture(textures_color[0], tex_coord_flipped);" << std::endl;
        sstr << "    texture_color_ambient_diffuse.a *= alpha_fac;" << std::endl;
    } else if (ntextures_color > 1) {
        if (alpha_threshold != 0) {
            throw std::runtime_error("Alpha-threshold not supported for multiple textures");
        }
    }
    if (alpha_threshold != 0) {
        if (ntextures_color == 0) {
            throw std::runtime_error("Alpha threshold requires texture");
        }
        sstr << "    if (texture_color_ambient_diffuse.a < " << alpha_threshold << ")" << std::endl;
        sstr << "        discard;" << std::endl;
    }
    if ((ntextures_color != 1) || !reorient_uv0) {
        compute_normal();
    }
    if (has_normalmap || has_interiormap) {
        sstr << "    vec3 tang = normalize(tangent);" << std::endl;
        sstr << "    vec3 bitan = normalize(bitangent);" << std::endl;
        sstr << "    mat3 TBN = mat3(tang, bitan, norm);" << std::endl;
    }
    if (has_interiormap) {
        sstr << "    if (is_in_interior(TBN)) {" << std::endl;
        sstr << "        return;" << std::endl;
        sstr << "    }" << std::endl;
    }
    if (ntextures_color > 1) {
        sstr << "    vec4 texture_color_ambient_diffuse = vec4(0, 0, 0, texture(textures_color[0], tex_coord_flipped * " << textures[0]->scale << ").a);" << std::endl;
        if (has_normalmap) {
            sstr << "    vec3 tnorm = vec3(0, 0, 0);" << std::endl;
        }
        sstr << "    float sum_weights = 0;" << std::endl;
        if (alpha_distances == default_linear_distances) {
            for (const BlendMapTexture* t : textures) {
                if (t->distances != default_linear_distances) {
                    if (orthographic) {
                        // throw std::runtime_error("Distances not supported by orthographic projection");
                        sstr << "    float dist = 1. / 0.;" << std::endl;
                    } else {
                        sstr << "    float dist = distance(viewPos, FragPos);" << std::endl;
                    }
                    break;
                }
            }
        }
        {
            size_t i = 0;
            for (const BlendMapTexture* t : textures) {
                sstr << "    {" << std::endl;
                if (t->cosines != default_linear_cosines) {
                    sstr << "        float cosine = dot(norm, vec3(" << t->normal(0) << ", " << t->normal(1) << ", " << t->normal(2) << "));" << std::endl;
                }
                std::list<std::string> checks;
                if (t->min_height != -INFINITY) {
                    checks.push_back("(FragPos.y >= " + std::to_string(t->min_height) + ')');
                }
                if (t->max_height != INFINITY) {
                    checks.push_back("(FragPos.y <= " + std::to_string(t->max_height) + ')');
                }
                if (t->distances(0) != 0) {
                    checks.push_back("(dist >= " + std::to_string(t->distances(0)) + ')');
                }
                if (t->distances(3) != INFINITY) {
                    checks.push_back("(dist <= " + std::to_string(t->distances(3)) + ')');
                }
                if (t->cosines(0) != -1) {
                    checks.push_back("(cosine >= " + std::to_string(t->cosines(0)) + ')');
                }
                if (t->cosines(3) != 1) {
                    checks.push_back("(cosine <= " + std::to_string(t->cosines(3)) + ')');
                }
                if (!checks.empty()) {
                    sstr << "        if (" << join(" && ", checks) << ") {" << std::endl;
                }
                sstr << "            float weight = " << t->weight << ";" << std::endl;
                sstr << "            float scale = " << t->scale << ';' << std::endl;
                if (t->distances(0) != t->distances(1)) {
                    sstr << "            if (dist <= " << t->distances(1) << ") {" << std::endl;
                    sstr << "                weight *= (dist - " << t->distances(0) << ") / " << (t->distances(1) - t->distances(0)) << ";" << std::endl;
                    sstr << "            }" << std::endl;
                }
                if (t->distances(3) != t->distances(2)) {
                    sstr << "            if (dist >= " << t->distances(2) << ") {" << std::endl;
                    sstr << "                weight *= (" << t->distances(3) << " - dist) / " << (t->distances(3) - t->distances(2)) << ";" << std::endl;
                    sstr << "            }" << std::endl;
                }
                if (t->cosines(0) != t->cosines(1)) {
                    sstr << "            if (cosine <= " << t->cosines(1) << ") {" << std::endl;
                    sstr << "                weight *= (cosine - " << t->cosines(0) << ") / " << (t->cosines(1) - t->cosines(0)) << ";" << std::endl;
                    sstr << "            }" << std::endl;
                }
                if (t->cosines(3) != t->cosines(2)) {
                    sstr << "            if (cosine >= " << t->cosines(2) << ") {" << std::endl;
                    sstr << "                weight *= (" << t->cosines(3) << " - cosine) / " << (t->cosines(3) - t->cosines(2)) << ";" << std::endl;
                    sstr << "            }" << std::endl;
                }
                if ((t->texture_descriptor.color_mode == ColorMode::RGBA) && (t->discreteness != 0)) {
                    sstr << "            vec4 bcolor = texture(textures_color[" << i << "], tex_coord_flipped * scale).rgba;" << std::endl;
                    sstr << "            weight *= clamp(0.5 + " << t->discreteness << " * (bcolor.a - 0.5), 0, 1);" << std::endl;
                    // sstr << "            weight *= bcolor.a;" << std::endl;
                } else if (
                    (t->texture_descriptor.color_mode == ColorMode::RGB) ||
                    ((t->texture_descriptor.color_mode == ColorMode::RGBA) && (t->discreteness == 0)))
                {
                    sstr << "            vec3 bcolor = texture(textures_color[" << i << "], tex_coord_flipped * scale).rgb;" << std::endl;
                } else {
                    throw std::runtime_error("Unsupported color mode: \"" + color_mode_to_string(t->texture_descriptor.color_mode) + '"');
                }
                sstr << "            texture_color_ambient_diffuse.rgb += weight * bcolor.rgb;" << std::endl;
                if (has_normalmap) {
                    if (t->texture_descriptor.normal.empty()) {
                        sstr << "            tnorm.z += weight;" << std::endl;
                    } else {
                        sstr << "            tnorm += weight * (2 * texture(texture_normalmap[" << i << "], tex_coord_flipped * scale).rgb - 1);" << std::endl;
                    }
                }
                sstr << "            sum_weights += weight;" << std::endl;
                if (!checks.empty()) {
                    sstr << "        }" << std::endl;
                }
                sstr << "    }" << std::endl;
                ++i;
            }
        }
        sstr << "    if (sum_weights < 1e-3) {" << std::endl;
        sstr << "        texture_color_ambient_diffuse.rgb = vec3(1, 0, 1);" << std::endl;
        sstr << "    } else {" << std::endl;
        sstr << "        texture_color_ambient_diffuse.rgb /= sum_weights;" << std::endl;
        sstr << "    }" << std::endl;
        // sstr << "    texture_color_ambient_diffuse.rgb /= max(1e-6, sum_weights);" << std::endl;
    }
    if (has_normalmap) {
        if (ntextures_color == 1) {
            sstr << "    vec3 tnorm = 2 * texture(texture_normalmap[0], tex_coord_flipped).rgb - 1;" << std::endl;
        }
        sstr << "    norm = normalize(TBN * tnorm);" << std::endl;
    }
    sstr << "    vec3 frag_brightness_emissive_ambient_diffuse = vec3(0, 0, 0);" << std::endl;
    sstr << "    vec3 frag_brightness_specular = vec3(0, 0, 0);" << std::endl;
    if (!ambience.all_equal(0) || !diffusivity.all_equal(0) || !specularity.all_equal(0)) {
        if (has_lightmap_color && !black_shadow_indices.empty()) {
            sstr << "    vec3 black_fac = vec3(1, 1, 1);" << std::endl;
        }
        assert_true(!(has_lightmap_color && has_lightmap_depth));
        if (has_lightmap_color && !black_shadow_indices.empty()) {
            sstr << "    {" << std::endl;
            for (size_t i : black_shadow_indices) {
                sstr << "        {" << std::endl;
                sstr << "            vec3 proj_coords11 = FragPosLightSpace[" << i << "].xyz / FragPosLightSpace[" << i << "].w;" << std::endl;
                sstr << "            vec3 proj_coords01 = proj_coords11 * 0.5 + 0.5;" << std::endl;
                sstr << "            black_fac = min(black_fac, texture(texture_light_color[" << i << "], proj_coords01.xy).rgb);" << std::endl;
                sstr << "        }" << std::endl;
            }
            sstr << "    }" << std::endl;
        }
        if (has_lightmap_depth) {
            sstr << "    for (int i = 0; i < " << lights.size() << "; ++i) {" << std::endl;
            sstr << "        vec3 proj_coords11 = FragPosLightSpace[i].xyz / FragPosLightSpace[i].w;" << std::endl;
            sstr << "        vec3 proj_coords01 = proj_coords11 * 0.5 + 0.5;" << std::endl;
            sstr << "        if (proj_coords01.z - 0.00002 < texture(texture_light_depth[i], proj_coords01.xy).r) {" << std::endl;
            if (!ambience.all_equal(0)) {
                sstr << "            frag_brightness_emissive_ambient_diffuse += phong_ambient(i);" << std::endl;
            }
            if (!diffusivity.all_equal(0)) {
                sstr << "            frag_brightness_emissive_ambient_diffuse += phong_diffuse(i, norm);" << std::endl;
            }
            if (!specularity.all_equal(0)) {
                sstr << "            frag_brightness_specular += phong_specular(i, norm);" << std::endl;
            }
            sstr << "        }" << std::endl;
            sstr << "    }" << std::endl;
        }
        if (!has_lightmap_depth && !light_shadow_indices.empty()) {
            for (size_t i : light_shadow_indices) {
                sstr << "        {" << std::endl;
                if (has_lightmap_color) {
                    sstr << "        vec3 proj_coords11 = FragPosLightSpace[" << i << "].xyz / FragPosLightSpace[" << i << "].w;" << std::endl;
                    sstr << "        vec3 proj_coords01 = proj_coords11 * 0.5 + 0.5;" << std::endl;
                    sstr << "        vec3 light_fac = texture(texture_light_color[" << i << "], proj_coords01.xy).rgb;" << std::endl;
                } else {
                    sstr << "        vec3 light_fac = vec3(1, 1, 1);" << std::endl;
                }
                if (!ambience.all_equal(0)) {
                    sstr << "        frag_brightness_emissive_ambient_diffuse += light_fac * phong_ambient(" << i << ");" << std::endl;
                }
                if (!diffusivity.all_equal(0)) {
                    sstr << "        frag_brightness_emissive_ambient_diffuse += light_fac * phong_diffuse(" << i << ", norm);" << std::endl;
                }
                if (!specularity.all_equal(0)) {
                    sstr << "        frag_brightness_specular += light_fac * phong_specular(" << i << ", norm);" << std::endl;
                }
                sstr << "    }" << std::endl;
            }
        }
        if (!light_noshadow_indices.empty()) {
            for (size_t i : light_noshadow_indices) {
                sstr << "    {" << std::endl;
                if (!ambience.all_equal(0)) {
                    sstr << "        frag_brightness_emissive_ambient_diffuse += phong_ambient(" << i << ");" << std::endl;
                }
                if (!diffusivity.all_equal(0)) {
                    sstr << "        frag_brightness_emissive_ambient_diffuse += phong_diffuse(" << i << ", norm);" << std::endl;
                }
                if (!specularity.all_equal(0)) {
                    sstr << "        frag_brightness_specular += phong_specular(" << i << ", norm);" << std::endl;
                }
                sstr << "    }" << std::endl;
            }
        }
    }
    if (has_specularmap) {
        if (textures.size() != 1) {
            throw std::runtime_error("Specular maps not supported for blended textures");
        }
        sstr << "    vec3 texture_specularity = texture(texture_specularmap, tex_coord_flipped).rgb;" << std::endl;
        sstr << "    frag_brightness_specular.rgb *= texture_specularity;" << std::endl;
    } else {
        sstr << "    vec3 texture_specularity = vec3(1, 1, 1);" << std::endl;
    }
    if (has_lightmap_color && !black_shadow_indices.empty()) {
        sstr << "    frag_brightness_emissive_ambient_diffuse *= black_fac;" << std::endl;
        sstr << "    frag_brightness_specular *= black_fac;" << std::endl;
    }
    if ((ntextures_color == 0) && has_dirtmap) {
        throw std::runtime_error("Combination of ((ntextures_color == 0) && has_dirtmap) is not supported");
    }
    if (has_dirtmap) {
        sstr << "    float dirtiness = texture(texture_dirtmap, tex_coord_dirtmap).r;" << std::endl;
        sstr << "    vec4 dirt_color = texture(texture_dirt, tex_coord_flipped * " << dirt_scale << " );" << std::endl;
        if (dirt_color_mode == ColorMode::RGBA) {
            sstr << "    dirtiness *= dirt_color.a;" << std::endl;
        } else if (dirt_color_mode != ColorMode::RGB) {
            throw std::runtime_error("Unsupported dirt color mode: " + color_mode_to_string(dirt_color_mode));
        }
        sstr << "    dirtiness += " << dirtmap_offset << ";" << std::endl;
        sstr << "    dirtiness = clamp(0.5 + " << dirtmap_discreteness << " * (dirtiness - 0.5), 0, 1);" << std::endl;
        // sstr << "    dirtiness += clamp(0.005 + 80 * (0.98 - norm.y), 0, 1);" << std::endl;
        sstr << "    frag_color.a = texture_color_ambient_diffuse.a;" << std::endl;
        sstr << "    frag_color.rgb = texture_color_ambient_diffuse.rgb * (1 - dirtiness)" << std::endl;
        sstr << "                     + dirt_color.rgb * dirtiness;" << std::endl;
        sstr << "    frag_color.rgb *= color;" << std::endl;
        sstr << "    frag_brightness_specular.rgb *= (1 - dirtiness);" << std::endl;
    } else if (ntextures_color != 0) {
        sstr << "    frag_color = texture_color_ambient_diffuse * vec4(color, 1.0);" << std::endl;
    } else {
        sstr << "    frag_color = vec4(color, alpha_fac);" << std::endl;
    }
    if (!emissivity.all_equal(0.f)) {
        sstr << "    frag_brightness_emissive_ambient_diffuse += vec3(" << emissivity(0) << ", " << emissivity(1) << ", " << emissivity(2) << ");" << std::endl;
    }
    sstr << "    frag_color.rgb *= frag_brightness_emissive_ambient_diffuse;" << std::endl;
    sstr << "    frag_color.rgb += frag_brightness_specular;" << std::endl;
    if (reflection_strength != 0.f) {
        if (!orthographic) {
            sstr << "    vec3 viewDir = normalize(viewPos - FragPos);" << std::endl;
        }
        if (reflect_only_y) {
            sstr << "    vec3 reflectedDir = R * reflect(-viewDir, R[1]);" << std::endl;
        } else {
            sstr << "    vec3 reflectedDir = R * reflect(-viewDir, norm);" << std::endl;
        }
        // Modification proposed in https://learnopengl.com/Advanced-OpenGL/Cubemaps#comment-5197766106
        // This works in combination with not flipping the y-coordinate when loading the texture.
        sstr << "    frag_color.rgb = (1 - " << reflection_strength << " * texture_specularity) * frag_color.rgb + " << reflection_strength << " * texture_specularity * texture(texture_reflection, vec3(reflectedDir.xy, -reflectedDir.z)).rgb;" << std::endl;
    }
    if (any(render_pass & ExternalRenderPassType::LIGHTMAP_BLOBS_MASK)) {
        // Do nothing (keep colors)
    } else if (any(render_pass & ExternalRenderPassType::LIGHTMAP_COLOR_MASK)) {
        sstr << "    frag_color.r = 0.5;" << std::endl;
        sstr << "    frag_color.g = 0.5;" << std::endl;
        sstr << "    frag_color.b = 0.5;" << std::endl;
    }
    sstr << "}" << std::endl;
    if (getenv_default_bool("PRINT_SHADERS", false)) {
        std::cerr << std::endl;
        std::cerr << std::endl;
        std::cerr << std::endl;
        std::cerr << "Fragment" << std::endl;
        if (!textures.empty()) {
            std::cerr << "Color: " + textures[0]->texture_descriptor.color << std::endl;
        }
        std::cerr << sstr.str() << std::endl;
    }
    return sstr.str();
}};

ColoredVertexArrayResource::ColoredVertexArrayResource(
    const std::shared_ptr<AnimatedColoredVertexArrays>& triangles,
    std::unique_ptr<Instances>&& instances)
: triangles_res_{triangles},
  scene_node_resources_{RenderingContextStack::primary_scene_node_resources()},
  rendering_resources_{RenderingContextStack::primary_rendering_resources()},
  instances_{std::move(instances)}
{
#ifdef DEBUG
    triangles_res_->check_consistency();
#endif
}

ColoredVertexArrayResource::ColoredVertexArrayResource(
    const std::list<std::shared_ptr<ColoredVertexArray<float>>>& striangles,
    const std::list<std::shared_ptr<ColoredVertexArray<double>>>& dtriangles,
    std::unique_ptr<Instances>&& instances)
: ColoredVertexArrayResource{
    std::make_shared<AnimatedColoredVertexArrays>(),
    std::move(instances)}
{
    triangles_res_->scvas = striangles;
    triangles_res_->dcvas = dtriangles;
#ifdef DEBUG
    triangles_res_->check_consistency();
#endif
}

ColoredVertexArrayResource::ColoredVertexArrayResource(
    const std::shared_ptr<ColoredVertexArray<float>>& striangles,
    std::unique_ptr<Instances>&& instances)
: ColoredVertexArrayResource(
    std::list<std::shared_ptr<ColoredVertexArray<float>>>{striangles},
    std::list<std::shared_ptr<ColoredVertexArray<double>>>{},
    std::move(instances))
{}

ColoredVertexArrayResource::ColoredVertexArrayResource(
    const std::shared_ptr<ColoredVertexArray<double>>& dtriangles,
    std::unique_ptr<Instances>&& instances)
: ColoredVertexArrayResource(
    std::list<std::shared_ptr<ColoredVertexArray<float>>>{},
    std::list<std::shared_ptr<ColoredVertexArray<double>>>{dtriangles},
    std::move(instances))
{}

ColoredVertexArrayResource::ColoredVertexArrayResource(const std::shared_ptr<AnimatedColoredVertexArrays>& triangles)
: ColoredVertexArrayResource(triangles, nullptr)
{}

ColoredVertexArrayResource::ColoredVertexArrayResource(
    const std::list<std::shared_ptr<ColoredVertexArray<float>>>& striangles)
: ColoredVertexArrayResource{striangles, std::list<std::shared_ptr<ColoredVertexArray<double>>>{}}
{}

ColoredVertexArrayResource::ColoredVertexArrayResource(
    const std::list<std::shared_ptr<ColoredVertexArray<double>>>& dtriangles)
: ColoredVertexArrayResource{std::list<std::shared_ptr<ColoredVertexArray<float>>>{}, dtriangles}
{}
    
ColoredVertexArrayResource::ColoredVertexArrayResource(
    const std::list<std::shared_ptr<ColoredVertexArray<float>>>& striangles,
    const std::list<std::shared_ptr<ColoredVertexArray<double>>>& dtriangles)
: ColoredVertexArrayResource(striangles, dtriangles, nullptr)
{}

ColoredVertexArrayResource::ColoredVertexArrayResource(
    const std::shared_ptr<ColoredVertexArray<float>>& striangles)
: ColoredVertexArrayResource(striangles, nullptr)
{}

ColoredVertexArrayResource::ColoredVertexArrayResource(
    const std::shared_ptr<ColoredVertexArray<double>>& dtriangles)
: ColoredVertexArrayResource(dtriangles, nullptr)
{}

ColoredVertexArrayResource::~ColoredVertexArrayResource()
{}

void ColoredVertexArrayResource::preload() const {
    auto preload_textures = [this](const auto& cvas) {
        for (auto& cva : cvas) {
            for (auto& t : cva->material.textures) {
                rendering_resources_->preload(t.texture_descriptor);
            }
        }
    };
    preload_textures(triangles_res_->scvas);
    preload_textures(triangles_res_->dcvas);
}

void ColoredVertexArrayResource::instantiate_renderable(const InstantiationOptions& options) const
{
#ifdef DEBUG
    triangles_res_->check_consistency();
#endif
    options.scene_node.add_renderable(options.instance_name, std::make_shared<RenderableColoredVertexArray>(
        shared_from_this(),
        options.renderable_resource_filter));
}

std::shared_ptr<AnimatedColoredVertexArrays> ColoredVertexArrayResource::get_animated_arrays() const {
    return triangles_res_;
}

void ColoredVertexArrayResource::generate_triangle_rays(size_t npoints, const FixedArray<float, 3>& lengths, bool delete_triangles) {
    auto gen_triangle_rays = [&]<typename TPos>(const std::list<std::shared_ptr<ColoredVertexArray<TPos>>>& cvas)
    {
        for (auto& t : cvas) {
            auto r = Mlib::generate_triangle_rays(t->triangles, npoints, lengths TEMPLATEV casted<TPos>());
            t->lines.reserve(t->lines.size() + r.size());
            for (const auto& l : r) {
                t->lines.push_back({
                    ColoredVertex<TPos>{
                        .position = l(0),
                        .color = {1.f, 1.f, 1.f},
                        .uv = {0.f, 0.f}
                    },
                    ColoredVertex<TPos>{
                        .position = l(1),
                        .color = {1.f, 1.f, 1.f},
                        .uv = {0.f, 1.f}
                    }
                });
            }
            if (delete_triangles) {
                t->triangles.clear();
            }
        }
    };
    gen_triangle_rays(triangles_res_->scvas);
    gen_triangle_rays(triangles_res_->dcvas);
}

void ColoredVertexArrayResource::generate_ray(const FixedArray<float, 3>& from, const FixedArray<float, 3>& to) {
    if ((triangles_res_->scvas.size() + triangles_res_->dcvas.size()) != 1) {
        throw std::runtime_error("generate_ray requires exactly one triangle mesh");
    }
    auto gen_ray = [&]<typename TPos>(const std::list<std::shared_ptr<ColoredVertexArray<TPos>>>& cvas)
    {
        cvas.front()->lines.push_back({
            ColoredVertex<TPos>{
                .position = from,
                .color = {1.f, 1.f, 1.f},
                .uv = {0.f, 0.f}
            },
            ColoredVertex<TPos>{
                .position = to,
                .color = {1.f, 1.f, 1.f},
                .uv = {0.f, 1.f}
            }
        });
    };
    if (!triangles_res_->scvas.empty()) {
        gen_ray(triangles_res_->scvas);
    }
    if (!triangles_res_->dcvas.empty()) {
        gen_ray(triangles_res_->scvas);
    }
}

std::shared_ptr<SceneNodeResource> ColoredVertexArrayResource::generate_grind_lines(
    float edge_angle,
    float averaged_normal_angle,
    const ColoredVertexArrayFilter& filter) const
{
    return std::make_shared<ColoredVertexArrayResource>(
        triangles_res_->generate_grind_lines(edge_angle, averaged_normal_angle, filter));
}

std::shared_ptr<SceneNodeResource> ColoredVertexArrayResource::generate_contour_edges() const {
    std::list<std::shared_ptr<ColoredVertexArray<float>>> dest_scvas;
    std::list<std::shared_ptr<ColoredVertexArray<double>>> dest_dcvas;
    for (auto& t : triangles_res_->scvas) {
        dest_scvas.push_back(std::make_shared<ColoredVertexArray<float>>(
            t->generate_contour_edges()));
    }
    for (auto& t : triangles_res_->dcvas) {
        dest_dcvas.push_back(std::make_shared<ColoredVertexArray<double>>(
            t->generate_contour_edges()));
    }
    return std::make_shared<ColoredVertexArrayResource>(dest_scvas, dest_dcvas);
}

// std::shared_ptr<SceneNodeResource> ColoredVertexArrayResource::extract_by_predicate(
//     const std::function<bool(const ColoredVertexArray& cva)>& predicate)
// {
//     std::list<std::shared_ptr<ColoredVertexArray>> dest_cvas;
//     for (auto it = triangles_res_->cvas.begin(); it != triangles_res_->cvas.end(); )
//     {
//         if (predicate(**it)) {
//             dest_cvas.splice(dest_cvas.end(), triangles_res_->cvas, it++);
//         } else {
//             ++it;
//         }
//     }
//     return std::make_shared<ColoredVertexArrayResource>(dest_cvas);
// }

// std::shared_ptr<SceneNodeResource> ColoredVertexArrayResource::copy_by_predicate(
//     const std::function<bool(const ColoredVertexArray& cva)>& predicate)
// {
//     std::list<std::shared_ptr<ColoredVertexArray>> dest_cvas;
//     for (const auto& cva : triangles_res_->cvas) {
//         if (predicate(*cva)) {
//             dest_cvas.push_back(cva);
//         }
//     }
//     return std::make_shared<ColoredVertexArrayResource>(dest_cvas);
// }

void ColoredVertexArrayResource::modify_physics_material_tags(
    PhysicsMaterial add,
    PhysicsMaterial remove,
    const ColoredVertexArrayFilter& filter)
{
    if (any(add & remove)) {
        throw std::runtime_error("Duplicate add/remove flags");
    }
    auto modify_tags = [&](auto& cvas){
        for (auto& cva : cvas) {
            if (filter.matches(*cva)) {
                cva->physics_material |= add;
                cva->physics_material &= ~remove;
            }
        }
    };
    modify_tags(triangles_res_->scvas);
    modify_tags(triangles_res_->dcvas);
}

void ColoredVertexArrayResource::downsample(size_t factor) {
    for (auto& t : triangles_res_->scvas) {
        t->downsample_triangles(factor);
    }
    for (auto& t : triangles_res_->dcvas) {
        t->downsample_triangles(factor);
    }
}

AggregateMode ColoredVertexArrayResource::aggregate_mode() const {
    std::set<AggregateMode> aggregate_modes;
    for (const auto& t : triangles_res_->scvas) {
        aggregate_modes.insert(t->material.aggregate_mode);
    }
    for (const auto& t : triangles_res_->dcvas) {
        aggregate_modes.insert(t->material.aggregate_mode);
    }
    if (aggregate_modes.empty()) {
        throw std::runtime_error("Cannot determine aggregate mode of empty array");
    }
    if (aggregate_modes.size() != 1) {
        throw std::runtime_error("aggregate_mode is not unique");
    }
    return *aggregate_modes.begin();
}

void ColoredVertexArrayResource::print(std::ostream& ostr) const {
    std::cerr << "ColoredVertexArrayResource\n";
    triangles_res_->print(ostr);
}

const ColoredRenderProgram& ColoredVertexArrayResource::get_render_program(
    const RenderProgramIdentifier& id,
    const std::vector<std::pair<TransformationMatrix<float, double, 3>, Light*>>& filtered_lights,
    const std::vector<size_t>& lightmap_indices,
    const std::vector<size_t>& light_noshadow_indices,
    const std::vector<size_t>& light_shadow_indices,
    const std::vector<size_t>& black_shadow_indices,
    const std::vector<BlendMapTexture*>& textures) const
{
    auto& rps = rendering_resources_->render_programs();
    {
        std::shared_lock lock{mutex_};
        if (auto it = rps.find(id); it != rps.end()) {
            return *it->second;
        }
    }
    std::unique_lock lock{mutex_};
    if (auto it = rps.find(id); it != rps.end()) {
        return *it->second;
    }
    auto rp = std::make_unique<ColoredRenderProgram>();
    assert_true(triangles_res_->bone_indices.empty() == !triangles_res_->skeleton);
    const char* vs_text = vertex_shader_text_gen(
        filtered_lights,
        textures,
        filtered_lights.size(),
        id.ntextures_color,
        id.nbillboard_ids,
        !id.lightmap_indices_color.empty(),
        !id.lightmap_indices_depth.empty(),
        id.ntextures_normal != 0,
        id.ntextures_reflection != 0,
        id.ntextures_dirt != 0,
        id.ntextures_interior != 0,
        !id.diffusivity.all_equal(0),
        !id.specularity.all_equal(0),
        id.has_instances,
        id.has_lookat,
        id.has_yangle,
        id.has_uv_offset_u,
        triangles_res_->bone_indices.size(),
        id.reorient_normals,
        id.reorient_uv0,
        id.orthographic,
        id.fragments_depend_on_distance,
        id.fragments_depend_on_normal);
    const char* fs_text = fragment_shader_text_textured_rgb_gen(
        filtered_lights,
        textures,
        light_noshadow_indices,
        light_shadow_indices,
        black_shadow_indices,
        filtered_lights.size(),
        id.ntextures_color,
        id.ntextures_normal,
        !id.lightmap_indices_color.empty(),
        !id.lightmap_indices_depth.empty(),
        id.has_specularmap,
        id.ntextures_normal != 0,
        id.nbillboard_ids,
        id.reflection_strength,
        id.reflect_only_y,
        id.ntextures_dirt != 0,
        id.ntextures_interior != 0,
        id.facade_edge_size,
        id.facade_inner_size,
        id.interior_size,
        id.dirt_color_mode,
        id.emissivity,
        id.ambience,
        id.diffusivity,
        id.specularity,
        (id.blend_mode == BlendMode::CONTINUOUS)
            ? id.alpha
            : 1.f,
        (id.blend_mode == BlendMode::OFF) ||
        (id.blend_mode == BlendMode::CONTINUOUS) ||
        (id.ntextures_color == 0)
            ? 0.f
            : (id.blend_mode == BlendMode::SEMI_CONTINUOUS)
                ? 0.2f
                : (id.blend_mode == BlendMode::BINARY_05)
                    ? 0.5f
                    : 1,
        id.alpha_distances,
        id.render_pass,
        id.reorient_normals,
        id.reorient_uv0,
        id.orthographic,
        id.fragments_depend_on_distance,
        id.fragments_depend_on_normal,
        id.dirtmap_offset,
        id.dirtmap_discreteness,
        id.dirt_scale);
    try {
        rp->allocate(vs_text, fs_text);

        rp->mvp_location = checked_glGetUniformLocation(rp->program, "MVP");
        if (id.has_uv_offset_u) {
            rp->uv_offset_u_location = checked_glGetUniformLocation(rp->program, "uv_offset_u");
        }
        for (size_t i = 0; i < id.ntextures_color; ++i) {
            rp->texture_color_locations[i] = checked_glGetUniformLocation(rp->program, ("textures_color[" + std::to_string(i) + "]").c_str());
        }
        if (!id.lightmap_indices_color.empty() || !id.lightmap_indices_depth.empty()) {
            for (size_t i = 0; i < filtered_lights.size(); ++i) {
                rp->mvp_light_locations[i] = checked_glGetUniformLocation(rp->program, ("MVP_light[" + std::to_string(i) + "]").c_str());
            }
        } else {
            // Do nothing
            // rp->mvp_light_location = 0;
        }
        if (id.nbillboard_ids != 0) {
            rp->vertex_scale_location = checked_glGetUniformLocation(rp->program, "vertex_scale");
            rp->uv_scale_location = checked_glGetUniformLocation(rp->program, "uv_scale");
            rp->uv_offset_location = checked_glGetUniformLocation(rp->program, "uv_offset");
            if (!id.orthographic) {
                rp->alpha_distances_location = checked_glGetUniformLocation(rp->program, "alpha_distances");
            } else {
                rp->alpha_distances_location = 0;
            }
        } else {
            rp->vertex_scale_location = 0;
            rp->uv_scale_location = 0;
            rp->uv_offset_location = 0;
            rp->alpha_distances_location = 0;
        }
        assert(id.lightmap_indices_color.empty() || id.lightmap_indices_depth.empty());
        if (!id.lightmap_indices_color.empty()) {
            for (size_t i : lightmap_indices) {
                rp->texture_lightmap_color_locations[i] = checked_glGetUniformLocation(rp->program, ("texture_light_color[" + std::to_string(i) + "]").c_str());
            }
        } else {
            // Do nothing
            // rp->texture_lightmap_color_location = 0;
        }
        if (!id.lightmap_indices_depth.empty()) {
            for (size_t i : lightmap_indices) {
                rp->texture_lightmap_depth_locations[i] = checked_glGetUniformLocation(rp->program, ("texture_light_depth[" + std::to_string(i) + "]").c_str());
            }
        } else {
            // Do nothing
            // rp->texture_lightmap_depth_location = 0;
        }
        if (id.ntextures_normal != 0) {
            size_t i = 0;
            for (const auto& r : textures) {
                if (!r->texture_descriptor.normal.empty()) {
                    rp->texture_normalmap_locations[i] = checked_glGetUniformLocation(rp->program, ("texture_normalmap[" + std::to_string(i) + "]").c_str());
                }
                ++i;
            }
        }
        if (id.ntextures_reflection != 0) {
            rp->texture_reflection_location = checked_glGetUniformLocation(rp->program, "texture_reflection");
        } else {
            rp->texture_reflection_location = 0;
        }
        if (id.ntextures_dirt != 0) {
            rp->mvp_dirtmap_location = checked_glGetUniformLocation(rp->program, "MVP_dirtmap");
            rp->texture_dirtmap_location = checked_glGetUniformLocation(rp->program, "texture_dirtmap");
            rp->texture_dirt_location = checked_glGetUniformLocation(rp->program, "texture_dirt");
        } else {
            rp->mvp_dirtmap_location = 0;
            rp->texture_dirtmap_location = 0;
            rp->texture_dirt_location = 0;
        }
        if (id.ntextures_interior != 0) {
            for (size_t i = 0; i < INTERIOR_COUNT; ++i) {
                rp->texture_interiormap_location(i) = checked_glGetUniformLocation(rp->program, ("texture_interior[" + std::to_string(i) + "]").c_str());
            }
        } else {
            rp->texture_interiormap_location = 0;
        }
        if (id.has_specularmap) {
            rp->texture_specularmap_location = checked_glGetUniformLocation(rp->program, "texture_specularmap");
        } else {
            rp->texture_specularmap_location = 0;
        }
        if (id.reflection_strength != 0.f) {
            rp->r_location = checked_glGetUniformLocation(rp->program, "R");
        } else {
            rp->r_location = 0;
        }
        {
            bool light_dir_required = !id.diffusivity.all_equal(0) || !id.specularity.all_equal(0);
            if (id.reorient_uv0 || light_dir_required || (id.fragments_depend_on_distance && !id.orthographic) || id.fragments_depend_on_normal || (id.ntextures_interior != 0)) {
                if (light_dir_required) {
                    for (size_t i = 0; i < filtered_lights.size(); ++i) {
                        if (!any(filtered_lights.at(i).second->shadow_render_pass & ExternalRenderPassType::LIGHTMAP_IS_BLACK_MASK)) {
                            rp->light_dir_locations[i] = checked_glGetUniformLocation(rp->program, ("lightDir[" + std::to_string(i) + "]").c_str());
                        }
                    }
                }
            }
        }
        // rp->light_position_location = checked_glGetUniformLocation(rp->program, "lightPos");
        assert_true(triangles_res_->bone_indices.empty() == !triangles_res_->skeleton);
        for (size_t i = 0; i < triangles_res_->bone_indices.size(); ++i) {
            rp->pose_positions[i] = checked_glGetUniformLocation(rp->program, ("bone_positions[" + std::to_string(i) + "]").c_str());
            rp->pose_quaternions[i] = checked_glGetUniformLocation(rp->program, ("bone_quaternions[" + std::to_string(i) + "]").c_str());
        }
        for (size_t i = 0; i < filtered_lights.size(); ++i) {
            if (!id.ambience.all_equal(0) && !any(filtered_lights.at(i).second->shadow_render_pass & ExternalRenderPassType::LIGHTMAP_IS_BLACK_MASK)) {
                rp->light_ambiences[i] = checked_glGetUniformLocation(rp->program, ("lightAmbience[" + std::to_string(i) + "]").c_str());
            }
            if (!id.diffusivity.all_equal(0) && !any(filtered_lights.at(i).second->shadow_render_pass & ExternalRenderPassType::LIGHTMAP_IS_BLACK_MASK)) {
                rp->light_diffusivities[i] = checked_glGetUniformLocation(rp->program, ("lightDiffusivity[" + std::to_string(i) + "]").c_str());
            }
            if (!id.specularity.all_equal(0) && !any(filtered_lights.at(i).second->shadow_render_pass & ExternalRenderPassType::LIGHTMAP_IS_BLACK_MASK)) {
                rp->light_specularities[i] = checked_glGetUniformLocation(rp->program, ("lightSpecularity[" + std::to_string(i) + "]").c_str());
            }
        }
        {
            bool pred0 = id.has_lookat || !id.specularity.all_equal(0) || (id.reflection_strength != 0.f) || id.reorient_uv0 || id.reorient_normals || (id.fragments_depend_on_distance && !id.orthographic);
            if (pred0 || (id.ntextures_interior != 0)) {
                if (pred0 && id.orthographic) {
                    rp->view_dir = checked_glGetUniformLocation(rp->program, "viewDir");
                    rp->view_pos = 0;
                } else {
                    rp->view_dir = 0;
                }
                if ((pred0 && !id.orthographic) || (id.ntextures_interior != 0)) {
                    rp->view_pos = checked_glGetUniformLocation(rp->program, "viewPos");
                } else {
                    rp->view_pos = 0;
                }
            } else {
                rp->view_dir = 0;
                rp->view_pos = 0;
            }
        }

        auto& result = *rp;
        rps.insert(std::make_pair(id, std::move(rp)));
        return result;
    } catch (const std::runtime_error& e) {
        std::string id;
        if (!textures.empty()) {
            id = "\nAmbient+diffuse: " + textures[0]->texture_descriptor.color;
        }
        throw std::runtime_error(
            std::string("Could not generate render program.\n") +
            e.what() +
            id +
            "\nVertex shader:\n" + vs_text +
            "\nFragment shader:\n" + fs_text);
    }
}

const SubstitutionInfo& ColoredVertexArrayResource::get_vertex_array(const std::shared_ptr<ColoredVertexArray<float>>& cva) const
{
    if ((cva->material.aggregate_mode != AggregateMode::NONE) && (instances_ == nullptr)) {
        throw std::runtime_error("get_vertex_array called on aggregated object \"" + cva->name + '"');
    }
    {
        std::shared_lock lock{mutex_};
        if (auto it = vertex_arrays_.find(cva.get()); it != vertex_arrays_.end()) {
            return *it->second;
        }
    }
    if (cva->triangles.empty()) {
        throw std::runtime_error("ColoredVertexArrayResource::get_vertex_array on empty array \"" + cva->name + '"');
    }
    std::unique_lock lock{mutex_};
    auto si = std::make_unique<SubstitutionInfo>();
    auto& va = si->va_;
    // https://stackoverflow.com/a/13405205/2292832
    CHK(glGenVertexArrays(1, &va.vertex_array));
    CHK(glBindVertexArray(va.vertex_array));

    CHK(glGenBuffers(1, &va.vertex_buffer));
    CHK(glBindBuffer(GL_ARRAY_BUFFER, va.vertex_buffer));
    CHK(glBufferData(GL_ARRAY_BUFFER, sizeof(cva->triangles[0]) * cva->triangles.size(), cva->triangles.data(), GL_STATIC_DRAW));

    ColoredVertex<float>* cv = nullptr;
    CHK(glEnableVertexAttribArray(IDX_POSITION));
    CHK(glVertexAttribPointer(IDX_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(ColoredVertex<float>), &cv->position));
    CHK(glEnableVertexAttribArray(IDX_COLOR));
    CHK(glVertexAttribPointer(IDX_COLOR, 3, GL_FLOAT, GL_FALSE, sizeof(ColoredVertex<float>), &cv->color));
    CHK(glEnableVertexAttribArray(IDX_UV));
    CHK(glVertexAttribPointer(IDX_UV, 2, GL_FLOAT, GL_FALSE, sizeof(ColoredVertex<float>), &cv->uv));
    // The vertex array is cached by cva => Use material properties, not the RenderProgramIdentifier.
    if (!cva->material.diffusivity.all_equal(0) || !cva->material.specularity.all_equal(0) || cva->material.fragments_depend_on_normal()) {
        CHK(glEnableVertexAttribArray(IDX_NORMAL));
        CHK(glVertexAttribPointer(IDX_NORMAL, 3, GL_FLOAT, GL_FALSE, sizeof(ColoredVertex<float>), &cv->normal));
    }
    // The vertex array is cached by cva => Use material properties, not the RenderProgramIdentifier.
    if (cva->material.has_normalmap() || !cva->material.interior_textures.empty()) {
        CHK(glEnableVertexAttribArray(IDX_TANGENT));
        CHK(glVertexAttribPointer(IDX_TANGENT, 3, GL_FLOAT, GL_FALSE, sizeof(ColoredVertex<float>), &cv->tangent));
    }
    if (instances_ != nullptr) {
        const std::vector<TransformationAndBillboardId>& inst = instances_->at(cva.get());
        if (inst.empty()) {
            throw std::runtime_error("ColoredVertexArrayResource::get_vertex_array received empty instances \"" + cva->name + '"');
        }
        if (cva->material.transformation_mode == TransformationMode::POSITION_YANGLE) {
            std::vector<FixedArray<float, 4>> positions;
            positions.reserve(inst.size());
            for (const TransformationAndBillboardId& m : inst) {
                positions.push_back(FixedArray<float, 4>{
                    m.transformation_matrix.t()(0),
                    m.transformation_matrix.t()(1),
                    m.transformation_matrix.t()(2),
                    std::atan2(-m.transformation_matrix.R()(2, 0), m.transformation_matrix.R()(0, 0))});
            }
            CHK(glGenBuffers(1, &va.position_buffer));
            CHK(glBindBuffer(GL_ARRAY_BUFFER, va.position_buffer));
            CHK(glBufferData(GL_ARRAY_BUFFER, sizeof(positions[0]) * positions.size(), positions.data(), GL_STATIC_DRAW));

            CHK(glEnableVertexAttribArray(IDX_INSTANCE_ATTRS));
            CHK(glVertexAttribPointer(IDX_INSTANCE_ATTRS, 4, GL_FLOAT, GL_FALSE, sizeof(positions[0]), nullptr));
            CHK(glVertexAttribDivisor(IDX_INSTANCE_ATTRS, 1));
        } else if ((cva->material.transformation_mode == TransformationMode::POSITION) ||
                   (cva->material.transformation_mode == TransformationMode::POSITION_LOOKAT))
        {
            std::vector<FixedArray<float, 3>> positions;
            positions.reserve(inst.size());
            for (const TransformationAndBillboardId& m : inst) {
                positions.push_back(m.transformation_matrix.t());
            }
            CHK(glGenBuffers(1, &va.position_buffer));
            CHK(glBindBuffer(GL_ARRAY_BUFFER, va.position_buffer));
            CHK(glBufferData(GL_ARRAY_BUFFER, sizeof(positions[0]) * positions.size(), positions.data(), GL_STATIC_DRAW));

            CHK(glEnableVertexAttribArray(IDX_INSTANCE_ATTRS));
            CHK(glVertexAttribPointer(IDX_INSTANCE_ATTRS, 3, GL_FLOAT, GL_FALSE, sizeof(positions[0]), nullptr));
            CHK(glVertexAttribDivisor(IDX_INSTANCE_ATTRS, 1));
        } else {
            throw std::runtime_error("Unsupported transformation mode for instances");
        }
        if (!cva->material.billboard_atlas_instances.empty()) {
            std::vector<uint32_t> billboard_ids;
            billboard_ids.reserve(inst.size());
            for (const TransformationAndBillboardId& m : inst) {
                if (m.billboard_id >= cva->material.billboard_atlas_instances.size()) {
                    throw std::runtime_error("Billboard ID too large");
                }
                billboard_ids.push_back(m.billboard_id);
            }
            CHK(glGenBuffers(1, &va.position_buffer));
            CHK(glBindBuffer(GL_ARRAY_BUFFER, va.position_buffer));
            CHK(glBufferData(GL_ARRAY_BUFFER, sizeof(billboard_ids[0]) * billboard_ids.size(), billboard_ids.data(), GL_STATIC_DRAW));

            CHK(glEnableVertexAttribArray(IDX_BILLBOARD_IDS));
            CHK(glVertexAttribIPointer(IDX_BILLBOARD_IDS, 1, GL_UNSIGNED_INT, sizeof(billboard_ids[0]), nullptr));
            CHK(glVertexAttribDivisor(IDX_BILLBOARD_IDS, 1));
        }
    }
    assert_true(cva->triangle_bone_weights.empty() == !triangles_res_->skeleton);
    if (triangles_res_->skeleton != nullptr) {
        std::vector<FixedArray<ShaderBoneWeight, 3>> triangle_bone_weights(cva->triangle_bone_weights.size());
        for (size_t tid = 0; tid < triangle_bone_weights.size(); ++tid) {
            const auto& td = cva->triangle_bone_weights[tid];  // std::vector of bone weights.
            auto& ts = triangle_bone_weights[tid];             // FixedArray of sorted bone weights.
            for (size_t vid = 0; vid < td.length(); ++vid) {
                auto vd = td(vid);   // Copy of std::vector of bone weights, to be sorted.
                auto& vs = ts(vid);  // Reference to FixedArray of sorted bone weights.
                // Sort in descending order
                std::sort(vd.begin(), vd.end(), [](const BoneWeight& w0, const BoneWeight& w1){return w0.weight > w1.weight;});
                float sum_weights = 0;
                for (size_t i = 0; i < ANIMATION_NINTERPOLATED; ++i) {
                    if (i < vd.size()) {
                        if (vd[i].bone_index >= triangles_res_->bone_indices.size()) {
                            throw std::runtime_error(
                                "Bone index too large in get_vertex_array: " +
                                std::to_string(vd[i].bone_index) + " >= " +
                                std::to_string(triangles_res_->bone_indices.size()));
                        }
                        if (vd[i].bone_index > 255) {
                            throw std::runtime_error("Bone index too large for unsigned char");
                        }
                        vs.indices[i] = (unsigned char)vd[i].bone_index;
                        vs.weights[i] = vd[i].weight;
                        sum_weights += vs.weights[i];
                    } else {
                        vs.indices[i] = 0;
                        vs.weights[i] = 0;
                    }
                }
                if (sum_weights < 1e-3) {
                    throw std::runtime_error("Sum of weights too small");
                }
                if (sum_weights > 1.1) {
                    throw std::runtime_error("Sum of weights too large");
                }
                for (size_t i = 0; i < ANIMATION_NINTERPOLATED; ++i) {
                    vs.weights[i] /= sum_weights;
                }
            }
        }
        CHK(glGenBuffers(1, &va.bone_weight_buffer));
        CHK(glBindBuffer(GL_ARRAY_BUFFER, va.bone_weight_buffer));
        CHK(glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_bone_weights[0]) * triangle_bone_weights.size(), triangle_bone_weights.data(), GL_STATIC_DRAW));

        ShaderBoneWeight* bw = nullptr;
        CHK(glEnableVertexAttribArray(IDX_BONE_INDICES));
        CHK(glVertexAttribIPointer(IDX_BONE_INDICES, ANIMATION_NINTERPOLATED, GL_UNSIGNED_BYTE, sizeof(ShaderBoneWeight), &bw->indices));
        CHK(glEnableVertexAttribArray(IDX_BONE_WEIGHTS));
        CHK(glVertexAttribPointer(IDX_BONE_WEIGHTS, ANIMATION_NINTERPOLATED, GL_FLOAT, GL_FALSE, sizeof(ShaderBoneWeight), &bw->weights));
    }
    if (!cva->material.interior_textures.empty()) {
        std::vector<ShaderInteriorMappedFacade> shader_interior_mapped_facade;
        shader_interior_mapped_facade.reserve(3 * cva->triangles.size());
        for (const auto& t : cva->triangles) {
            for (size_t i = 0; i < 3; ++i) {
                shader_interior_mapped_facade.push_back(ShaderInteriorMappedFacade{
                    .bottom_left = t(0).position,
                    .multiplier = FixedArray<float, 2>{1.f, 1.f}
                });
            }
        }
        CHK(glGenBuffers(1, &va.interior_mapping_buffer));
        CHK(glBindBuffer(GL_ARRAY_BUFFER, va.interior_mapping_buffer));
        CHK(glBufferData(GL_ARRAY_BUFFER, sizeof(shader_interior_mapped_facade[0]) * shader_interior_mapped_facade.size(), shader_interior_mapped_facade.data(), GL_STATIC_DRAW));

        ShaderInteriorMappedFacade* im = nullptr;
        CHK(glEnableVertexAttribArray(IDX_INTERIOR_MAPPING_BOTTOM_LEFT));
        CHK(glVertexAttribPointer(IDX_INTERIOR_MAPPING_BOTTOM_LEFT, 3, GL_FLOAT, GL_FALSE, sizeof(ShaderInteriorMappedFacade), &im->bottom_left));
        CHK(glEnableVertexAttribArray(IDX_INTERIOR_MAPPING_MULTIPLIER));
        CHK(glVertexAttribPointer(IDX_INTERIOR_MAPPING_MULTIPLIER, 2, GL_FLOAT, GL_FALSE, sizeof(ShaderInteriorMappedFacade), &im->multiplier));
    }

    CHK(glBindVertexArray(0));
    si->cva_ = cva;
    si->ntriangles_ = cva->triangles.size();
    si->nlines_ = cva->lines.size();
    auto& result = *si;  // store data before std::move (unique_ptr)
    vertex_arrays_.insert(std::make_pair(cva.get(), std::move(si)));
    return result;
}

void ColoredVertexArrayResource::set_absolute_joint_poses(
    const std::vector<OffsetAndQuaternion<float, float>>& poses)
{
    for (auto& t : triangles_res_->scvas) {
        t = t->transformed<float>(poses, "_transformed_oq");
    }
    if (!triangles_res_->dcvas.empty()) {
        throw std::runtime_error("Poses only support for single precision");
    }
}

void ColoredVertexArrayResource::import_bone_weights(
    const AnimatedColoredVertexArrays& other_acvas,
    float max_distance)
{
    Mlib::import_bone_weights(
        *triangles_res_,
        other_acvas,
        max_distance);
}
