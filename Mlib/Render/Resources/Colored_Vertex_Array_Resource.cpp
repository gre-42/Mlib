#include "Colored_Vertex_Array_Resource.hpp"
#include <Mlib/Assert.hpp>
#include <Mlib/Default_Uninitialized_Vector.hpp>
#include <Mlib/Env.hpp>
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Geometry/Instance/Rendering_Dynamics.hpp>
#include <Mlib/Geometry/Interfaces/IIntersectable.hpp>
#include <Mlib/Geometry/Material/Blend_Distances.hpp>
#include <Mlib/Geometry/Material/Blend_Map_Texture.hpp>
#include <Mlib/Geometry/Material/Interior_Texture_Set.hpp>
#include <Mlib/Geometry/Material/Particle_Type.hpp>
#include <Mlib/Geometry/Material_Features.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array_Filter.hpp>
#include <Mlib/Geometry/Mesh/Import_Bone_Weights.hpp>
#include <Mlib/Geometry/Mesh/Modifiers/Cluster_Meshes.hpp>
#include <Mlib/Geometry/Mesh/Modifiers/Mesh_And_Position.hpp>
#include <Mlib/Geometry/Mesh/Modifiers/Split_Meshes.hpp>
#include <Mlib/Geometry/Mesh/Transformation_And_Billboard_Id.hpp>
#include <Mlib/Geometry/Mesh/Triangle_Rays.hpp>
#include <Mlib/Geometry/Mesh/Typed_Mesh.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Images/Coordinates_Fixed.hpp>
#include <Mlib/Images/Revert_Axis.hpp>
#include <Mlib/Images/Vectorial_Pixels.hpp>
#include <Mlib/Iterator/Enumerate.hpp>
#include <Mlib/Map/Map.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Transformation/Translation_Matrix.hpp>
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Context_Query.hpp>
#include <Mlib/Render/Deallocate/Render_Deallocator.hpp>
#include <Mlib/Render/Gen_Shader_Text.hpp>
#include <Mlib/Render/Instance_Handles/Colored_Render_Program.hpp>
#include <Mlib/Render/Instance_Handles/IArray_Buffer.hpp>
#include <Mlib/Render/Renderables/Renderable_Colored_Vertex_Array.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Resource_Managers/Rendering_Resources.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/Distant_Triangle_Hider.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/IInstance_Buffers.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/IVertex_Data.hpp>
#include <Mlib/Render/Shader_Version_3_0.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Culling/Visibility_Check.hpp>
#include <Mlib/Scene_Graph/Elements/Light.hpp>
#include <Mlib/Scene_Graph/Elements/Make_Scene_Node.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Elements/Skidmark.hpp>
#include <Mlib/Scene_Graph/Instantiation/Child_Instantiation_Options.hpp>
#include <Mlib/Scene_Graph/Instantiation/IInstantiation_Reference.hpp>
#include <Mlib/Scene_Graph/Instantiation/Root_Instantiation_Options.hpp>
#include <Mlib/Scene_Graph/Interfaces/IImposters.hpp>
#include <Mlib/Scene_Graph/Resources/Renderable_Resource_Filter.hpp>
#include <Mlib/Stats/Mean.hpp>
#include <Mlib/Strings/String.hpp>
#include <iostream>
#include <mutex>

using namespace Mlib;

template <class T>
class NotSortedArray {
public:
    NotSortedArray(const T& value)
        : value_{ value }
    {}
    decltype(auto) begin() const {
        return value_.begin();
    }
    decltype(auto) end() const {
        return value_.end();
    }
    operator const T&() const {
        return value_;
    }
    const T& operator * () const {
        return value_;
    }
    const T* operator -> () const {
        return &value_;
    }
    size_t size() const {
        return value_.size();
    }
    bool empty() const {
        return value_.empty();
    }
    decltype(auto) operator [] (size_t i) const {
        return value_[i];
    }
    decltype(auto) at(size_t i) const {
        return value_.at(i);
    }
    std::strong_ordering operator <=> (const NotSortedArray&) const {
        return std::strong_ordering::equal;
    }
private:
    const T& value_;
};

struct UvMapKey {
    BlendMapUvSource uv_source;
    OrderableFixedArray<float, 2> offset;
    OrderableFixedArray<float, 2> scale;
    std::strong_ordering operator <=> (const UvMapKey&) const = default;
};

class NotSortedUvMap {
public:
    NotSortedUvMap(Map<UvMapKey, size_t>& m): m_{ m }
    {}
    decltype(auto) begin() const {
        return m_.begin();
    }
    decltype(auto) end() const {
        return m_.end();
    }
    void insert(const BlendMapTexture& t) const {
        m_.try_emplace({ t.uv_source, t.offset, t.scale }, m_.size());
    }
    size_t operator [] (const BlendMapTexture& t) const {
        return m_.at({ t.uv_source, t.offset, t.scale });
    }
    size_t size() const {
        return m_.size();
    }
    std::strong_ordering operator <=> (const NotSortedUvMap&) const {
        return std::strong_ordering::equal;
    }
private:
    Map<UvMapKey, size_t>& m_;
};

template <class T>
class NotSortedStruct {
public:
    NotSortedStruct(const T& value)
        : value_{ value }
    {}
    operator const T&() const {
        return value_;
    }
    const T& operator * () const {
        return value_;
    }
    const T* operator -> () const {
        return &value_;
    }
    std::strong_ordering operator <=> (const NotSortedStruct&) const {
        return std::strong_ordering::equal;
    }
private:
    const T& value_;
};

static const size_t ANIMATION_NINTERPOLATED = 4;
struct ShaderBoneWeight {
    unsigned char indices[ANIMATION_NINTERPOLATED];
    float weights[ANIMATION_NINTERPOLATED];
};

struct ShaderInteriorMappedFacade {
    FixedArray<float, 3> bottom_left;
    FixedArray<float, 4> uvmap;
};

enum class ReductionTarget {
    COLOR,
    ALPHA
};

namespace Mlib {

struct AttributeIndices {
    GLuint idx_position;
    GLuint idx_color;
    GLuint idx_normal;
    GLuint idx_tangent;
    GLuint idx_instance_attrs;
    GLuint idx_rotation_quaternion;
    GLuint idx_billboard_ids;
    GLuint idx_bone_indices;
    GLuint idx_bone_weights;
    GLuint idx_texture_layer;
    GLuint idx_interior_mapping_bottom_left;
    GLuint idx_interior_mapping_uvmap;
    GLuint idx_uv_0;
    GLuint idx_uv_1;
    GLuint uv_count;
    GLuint idx_cweight_0;
    GLuint cweight_count;
    GLuint idx_alpha;
};

struct AttributeIndexCalculator {
    bool has_position;
    bool has_color;
    bool has_normal;
    bool has_tangent;
    bool has_instance_attrs;
    bool has_rotation_quaternion;
    bool has_billboard_ids;
    bool has_bone_indices;
    bool has_bone_weights;
    bool has_texture_layer;
    bool has_interior_mapping_bottom_left;
    bool has_interior_mapping_multiplier;
    size_t nuvs;
    size_t ncweights;
    bool has_alpha;

    AttributeIndices build() const {
        AttributeIndices result;
        result.idx_position = 0;
        result.idx_color = result.idx_position + has_position;
        result.idx_normal = result.idx_color + has_color;
        result.idx_tangent = result.idx_normal + has_normal;
        result.idx_instance_attrs = result.idx_tangent + has_tangent;
        result.idx_rotation_quaternion = result.idx_instance_attrs + has_instance_attrs;
        result.idx_billboard_ids = result.idx_rotation_quaternion + has_rotation_quaternion;
        result.idx_bone_indices = result.idx_billboard_ids + has_billboard_ids;
        result.idx_bone_weights = result.idx_bone_indices + has_bone_indices;
        result.idx_texture_layer = result.idx_bone_weights + has_bone_weights;
        result.idx_interior_mapping_bottom_left = result.idx_texture_layer + has_texture_layer;
        result.idx_interior_mapping_uvmap = result.idx_interior_mapping_bottom_left + has_interior_mapping_bottom_left;
        result.idx_uv_0 = result.idx_interior_mapping_uvmap + has_interior_mapping_multiplier;
        result.idx_uv_1 = result.idx_uv_0 + 1;
        result.uv_count = integral_cast<GLuint>(nuvs);
        result.idx_cweight_0 = result.idx_uv_0 + result.uv_count;
        result.cweight_count = integral_cast<GLuint>(ncweights);
        result.idx_alpha = result.idx_cweight_0 + result.cweight_count;
        return result;
    }
};

}

static GenShaderText vertex_shader_text_gen{[](
    const NotSortedArray<std::vector<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<Light>>>>& lights,
    const NotSortedArray<std::vector<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<Skidmark>>>>& skidmarks,
    const NotSortedArray<std::vector<BlendMapTextureAndId>>& textures_color,
    const NotSortedArray<std::vector<BlendMapTextureAndId>>& textures_alpha,
    const NotSortedArray<std::vector<size_t>>& lightmap_indices,
    const NotSortedStruct<AttributeIndexCalculator>& attr_idc,
    const NotSortedStruct<AttributeIndices>& attr_ids,
    const NotSortedUvMap& uv_map,
    size_t nuv_indices,
    size_t ncweight_indices,
    bool has_alpha,
    size_t texture_modifiers_hash,
    size_t lights_hash,
    size_t skidmarks_hash,
    BillboardId nbillboard_ids,
    const OrderableFixedArray<float, 3>& reflectance,
    bool reflect_only_y,
    bool has_lightmap_color,
    bool has_lightmap_depth,
    bool has_normalmap,
    bool has_reflection_map,
    bool has_dirtmap,
    bool has_interiormap,
    bool has_horizontal_detailmap,
    bool has_diffusivity,
    bool has_nontrivial_specularity,
    bool has_fresnel_exponent,
    bool has_instances,
    bool has_lookat,
    bool has_yangle,
    bool has_rotation_quaternion,
    bool has_uv_offset_u,
    size_t nbones,
    TextureLayerProperties texture_layer_properties,
    bool reorient_normals,
    bool reorient_uv0,
    bool has_depth_fog,
    bool orthographic,
    bool fragments_depend_on_distance,
    bool fragments_depend_on_normal,
    float dirt_scale)
{
    auto tex_coords = [&nuv_indices](const UvMapKey& t) {
        std::stringstream sstr;
        sstr << std::scientific;
        sstr << '(';
        if (!t.offset.all_equal(0.f)) {
            sstr << "vec2(" << t.offset(0) << ", " << t.offset(1) << ") + ";
        }
        static_assert(BlendMapUvSource::VERTICAL_LAST == BlendMapUvSource::VERTICAL4);
        switch (t.uv_source) {
            case BlendMapUvSource::VERTICAL0:
            case BlendMapUvSource::VERTICAL1:
            case BlendMapUvSource::VERTICAL2:
            case BlendMapUvSource::VERTICAL3:
            case BlendMapUvSource::VERTICAL4:
            {
                auto id = t.uv_source - BlendMapUvSource::VERTICAL0;
                if (id >= nuv_indices) {
                    THROW_OR_ABORT("UV index too large");
                }
                sstr << "vTexCoord" << id;
                break;
            }
            case BlendMapUvSource::HORIZONTAL_XZ:
                sstr << "(vPosInstance.xz + horizontal_detailmap_remainder)";
                break;
            case BlendMapUvSource::HORIZONTAL_XY:
                sstr << "(vPosInstance.xy + horizontal_detailmap_remainder)";
                break;
            default:
                THROW_OR_ABORT("Unknown blend-map UV source");
        }
        if (t.scale.all_equal(1.f)) {
            sstr << ')';
        } else if (t.scale(0) == t.scale(1)) {
            sstr << " * " << t.scale(0) << ')';
        } else {
            sstr << " * vec2(" << t.scale(0) << ", " << t.scale(1) << "))";
        }
        return sstr.str();
    };
    std::stringstream sstr;
    sstr << std::scientific;
    sstr << SHADER_VER;
    sstr << "uniform mat4 MVP;" << std::endl;
    sstr << "layout (location=" << attr_ids->idx_position << ") in vec3 vPos;" << std::endl;
    sstr << "layout (location=" << attr_ids->idx_color << ") in vec3 vCol;" << std::endl;
    assert_true(attr_idc->nuvs == attr_ids->uv_count);
    assert_true(
        (attr_idc->nuvs == nuv_indices) ||
        ((nuv_indices == 1) && (attr_idc->nuvs != 0)));
    for (size_t i = 0; i < nuv_indices; ++i) {
        sstr << "layout (location=" << (attr_ids->idx_uv_0 + i) << ") in vec2 vTexCoord" << i << ";" << std::endl;
    }
    if (ncweight_indices > attr_ids->cweight_count) {
        THROW_OR_ABORT("CWeight index too large");
    }
    assert_true(attr_idc->ncweights == ncweight_indices);
    for (size_t i = 0; i < ncweight_indices; ++i) {
        sstr << "layout (location=" << (attr_ids->idx_cweight_0 + i) << ") in float vCWeight" << i << ";" << std::endl;
    }
    assert_true(attr_idc->has_alpha == has_alpha);
    if (has_alpha) {
        sstr << "layout (location=" << (attr_ids->idx_alpha) << ") in float vAlpha;" << std::endl;
    }
    if (reorient_uv0 || has_diffusivity || has_nontrivial_specularity || has_fresnel_exponent || has_interiormap || fragments_depend_on_normal || (!reflectance.all_equal(0.f) && !reflect_only_y)) {
        assert_true(attr_idc->has_normal);
        sstr << "layout (location=" << attr_ids->idx_normal << ") in vec3 vNormal;" << std::endl;
    }
    if (has_normalmap || has_interiormap) {
        assert_true(attr_idc->has_tangent);
        sstr << "layout (location=" << attr_ids->idx_tangent << ") in vec3 vTangent;" << std::endl;
    }
    assert_true(attr_idc->has_instance_attrs == has_instances);
    if (attr_idc->has_instance_attrs) {
        if (has_yangle) {
            sstr << "layout (location=" << attr_ids->idx_instance_attrs << ") in vec4 instancePosition;" << std::endl;
        } else {
            sstr << "layout (location=" << attr_ids->idx_instance_attrs << ") in vec3 instancePosition;" << std::endl;
        }
        if (has_rotation_quaternion) {
            assert_true(attr_idc->has_rotation_quaternion);
            sstr << "layout (location=" << attr_ids->idx_rotation_quaternion << ") in vec4 rotationQuaternion;" << std::endl;
        }
    } else if (has_lookat && !orthographic) {
        sstr << "const vec3 instancePosition = vec3(0.0, 0.0, 0.0);" << std::endl;
    }
    assert_true(attr_idc->has_billboard_ids == (nbillboard_ids != 0));
    if (attr_idc->has_billboard_ids) {
        sstr << "layout (location=" << attr_ids->idx_billboard_ids << ") in mediump uint billboard_id;" << std::endl;
        sstr << "uniform vec3 vertex_scale[" << nbillboard_ids << "];" << std::endl;
        sstr << "uniform vec2 uv_scale[" << nbillboard_ids << "];" << std::endl;
        sstr << "uniform vec2 uv_offset[" << nbillboard_ids << "];" << std::endl;
        if (any(texture_layer_properties & TextureLayerProperties::ATLAS)) {
            sstr << "uniform uint texture_layers[" << nbillboard_ids << "];" << std::endl;
        }
        if (!orthographic) {
            sstr << "uniform vec4 alpha_distances[" << nbillboard_ids << "];" << std::endl;
            sstr << "out float alpha_fac_v;" << std::endl;
        }
    } else if (has_alpha) {
        sstr << "out float alpha_fac_v;" << std::endl;
    }
    assert_true(attr_idc->has_bone_indices == (nbones != 0));
    assert_true(attr_idc->has_bone_weights == (nbones != 0));
    if (nbones != 0) {
        sstr << "layout (location=" << attr_ids->idx_bone_indices << ") in lowp uvec" << ANIMATION_NINTERPOLATED << " bone_ids;" << std::endl;
        sstr << "layout (location=" << attr_ids->idx_bone_weights << ") in vec" << ANIMATION_NINTERPOLATED << " bone_weights;" << std::endl;
        sstr << "uniform vec3 bone_positions[" << nbones << "];" << std::endl;
        sstr << "uniform vec4 bone_quaternions[" << nbones << "];" << std::endl;
    }
    assert_true(attr_idc->has_texture_layer == any(texture_layer_properties & TextureLayerProperties::VERTEX));
    if (any(texture_layer_properties & TextureLayerProperties::VERTEX)) {
        if (any(texture_layer_properties & TextureLayerProperties::DISCRETE)) {
            sstr << "layout (location=" << attr_ids->idx_texture_layer << ") in lowp uint texture_layer;" << std::endl;
        }
        if (any(texture_layer_properties & TextureLayerProperties::CONTINUOUS)) {
            sstr << "layout (location=" << attr_ids->idx_texture_layer << ") in float texture_layer;" << std::endl;
        }
    }
    if (has_uv_offset_u) {
        sstr << "uniform float uv_offset_u;" << std::endl;
    }
    if (has_horizontal_detailmap) {
        sstr << "uniform vec2 horizontal_detailmap_remainder;" << std::endl;
    }
    sstr << "out vec3 color;" << std::endl;
    for (size_t i = 0; i < uv_map.size(); ++i) {
        sstr << "out vec2 tex_coord" << i << ";" << std::endl;
    }
    for (size_t i = 0; i < ncweight_indices; ++i) {
        sstr << "out float cweight" << i << ";" << std::endl;
    }
    if (has_lightmap_color || has_lightmap_depth) {
        if (lights.empty()) {
            THROW_OR_ABORT("No lights despite has_lightmap_color or has_lightmap_depth");
        }
        for (size_t i : lightmap_indices) {
            sstr << "uniform mat4 MVP_light" << i << ";" << std::endl;
            if (i > lights.size()) {
                THROW_OR_ABORT("Light index out of bounds");
            }
            const auto& l = *lights[i].second;
            if (!l.vp.has_value()) {
                THROW_OR_ABORT("Light has no projection matrix");
            }
            if (has_lightmap_depth || !VisibilityCheck{*l.vp}.orthographic()) {
                // vec4 to avoid clipping problems
                sstr << "out vec4 FragPosLightSpace" << i << ";" << std::endl;
            } else {
                sstr << "out vec2 proj_coords01_light" << i << ";" << std::endl;
            }
        }
    }
    if (skidmarks.size() != 0) {
        sstr << "uniform mat4 MVP_skidmarks[" << skidmarks.size() << "];" << std::endl;
        sstr << "out vec2 proj_coords01_skidmarks[" << skidmarks.size() << "];" << std::endl;
    }
    if (has_normalmap || has_interiormap) {
        sstr << "out vec3 tangent;" << std::endl;
        sstr << "out vec3 bitangent;" << std::endl;
    }
    if (has_dirtmap) {
        sstr << "uniform mat4 MVP_dirtmap;" << std::endl;
        sstr << "out vec2 tex_coord_dirtmap;" << std::endl;
        sstr << "out vec2 tex_coord_dirt;" << std::endl;
    }
    if (any(texture_layer_properties & (TextureLayerProperties::VERTEX | TextureLayerProperties::ATLAS))) {
        if (any(texture_layer_properties & TextureLayerProperties::CONTINUOUS)) {
            sstr << "out float texture_layer_fs;" << std::endl;
        }
        if (any(texture_layer_properties & TextureLayerProperties::DISCRETE)) {
            sstr << "flat out lowp uint texture_layer_fs;" << std::endl;
        }
    }
    if (has_interiormap) {
        sstr << "layout (location=" << attr_ids->idx_interior_mapping_bottom_left << ") in vec3 interior_bottom_left;" << std::endl;
        sstr << "layout (location=" << attr_ids->idx_interior_mapping_uvmap << ") in vec4 interior_uvmap;" << std::endl;
        sstr << "out vec3 interior_bottom_left_fs;" << std::endl;
        sstr << "out vec4 interior_uvmap_fs;" << std::endl;
    }
    if (reorient_uv0 || reorient_normals || has_depth_fog || has_nontrivial_specularity || ((fragments_depend_on_distance || has_fresnel_exponent) && !orthographic) || has_interiormap || has_horizontal_detailmap || has_reflection_map) {
        sstr << "out highp vec3 FragPos;" << std::endl;
    }
    if (reorient_uv0 || has_diffusivity || has_nontrivial_specularity || has_fresnel_exponent || has_interiormap || fragments_depend_on_normal || (!reflectance.all_equal(0.f) && !reflect_only_y)) {
        sstr << "out vec3 Normal;" << std::endl;
    }
    if (orthographic) {
        if (has_lookat) {
            sstr << "uniform vec3 viewDir;" << std::endl;
        }
    } else if (has_lookat || (nbillboard_ids != 0)) {
        sstr << "uniform vec3 viewPos;" << std::endl;
    }
    if ((nbones != 0) || has_rotation_quaternion) {
        sstr << "vec3 rotate(vec4 v, vec3 p) {" << std::endl;
        sstr << "    return p + 2.0 * cross(v.xyz, cross(v.xyz, p) + v.w * p);" << std::endl;
        sstr << "}" << std::endl;
    }
    sstr << "void main()" << std::endl;
    sstr << "{" << std::endl;
    if (any(texture_layer_properties & TextureLayerProperties::ATLAS)) {
        sstr << "    texture_layer_fs = texture_layers[billboard_id];" << std::endl;
    }
    if (any(texture_layer_properties & TextureLayerProperties::VERTEX)) {
        sstr << "    texture_layer_fs = texture_layer;" << std::endl;
    }
    if (has_interiormap) {
        sstr << "    interior_bottom_left_fs = interior_bottom_left;" << std::endl;
        sstr << "    interior_uvmap_fs = interior_uvmap;" << std::endl;
    }
    sstr << "    vec3 vPosInstance;" << std::endl;
    if (reorient_uv0 || has_diffusivity || has_nontrivial_specularity || has_fresnel_exponent || has_interiormap || fragments_depend_on_normal || (!reflectance.all_equal(0.f) && !reflect_only_y)) {
        sstr << "    vec3 vNormalInstance;" << std::endl;
    }
    if (nbones != 0) {
        sstr << "    vPosInstance = vec3(0.0, 0.0, 0.0);" << std::endl;
        if (reorient_uv0 || has_diffusivity || has_nontrivial_specularity || has_fresnel_exponent || has_interiormap || fragments_depend_on_normal || (!reflectance.all_equal(0.f) && !reflect_only_y)) {
            sstr << "    vNormalInstance = vNormal;" << std::endl;
        }
        for (size_t k = 0; k < ANIMATION_NINTERPOLATED; ++k) {
            static std::map<unsigned char, char> m{
                {(unsigned char)0, 'x'},
                {(unsigned char)1, 'y'},
                {(unsigned char)2, 'z'},
                {(unsigned char)3, 'w'}};
            sstr << "    {" << std::endl;
            sstr << "        lowp uint i = bone_ids." << m.at((unsigned char)k) << ';' << std::endl;
            sstr << "        float weight = bone_weights." << m.at((unsigned char)k) << ';' << std::endl;
            sstr << "        vec3 o = bone_positions[i];" << std::endl;
            sstr << "        vec4 v = bone_quaternions[i];" << std::endl;
            sstr << "        vec3 p = vPos;" << std::endl;
            sstr << "        vPosInstance += weight * (o + rotate(v, p));" << std::endl;
            sstr << "    }" << std::endl;
        }
    } else {
        sstr << "    vPosInstance = vPos;" << std::endl;
        if (reorient_uv0 || has_diffusivity || has_nontrivial_specularity || has_fresnel_exponent || has_interiormap || fragments_depend_on_normal || (!reflectance.all_equal(0.f) && !reflect_only_y)) {
            sstr << "    vNormalInstance = vNormal;" << std::endl;
        }
    }
    if (nbillboard_ids != 0) {
        sstr << "    vPosInstance *= vertex_scale[billboard_id];" << std::endl;
    }
    for (size_t i = 0; i < ncweight_indices; ++i) {
        sstr << "    cweight" << i << " = vCWeight" << i << ";" << std::endl;
    }
    // if (has_lookat && !has_instances) {
    //     THROW_OR_ABORT("has_lookat requires has_instances");
    // }
    if (has_yangle && !has_instances) {
        THROW_OR_ABORT("has_yangle requires has_instances");
    }
    if (has_lookat || has_yangle || has_rotation_quaternion) {
        if (has_rotation_quaternion) {
            sstr << "    vPosInstance = rotate(rotationQuaternion, vPosInstance);" << std::endl;
        } else {
            sstr << "    mat3 lookat;" << std::endl;
            if (has_yangle) {
                sstr << "    vec2 dz_xz = vec2(sin(instancePosition.w), cos(instancePosition.w));" << std::endl;
            } else if (orthographic) {
                sstr << "    vec2 dz_xz = normalize(viewDir.xz);" << std::endl;
            } else {
                sstr << "    vec2 dz_xz = normalize(viewPos.xz - instancePosition.xz);" << std::endl;
            }
            sstr << "    vec3 dz = vec3(dz_xz.x, 0.0, dz_xz.y);" << std::endl;
            sstr << "    vec3 dy = vec3(0.0, 1.0, 0.0);" << std::endl;
            sstr << "    vec3 dx = normalize(cross(dy, dz));" << std::endl;
            sstr << "    lookat[0] = dx;" << std::endl;
            sstr << "    lookat[1] = dy;" << std::endl;
            sstr << "    lookat[2] = dz;" << std::endl;
            sstr << "    vPosInstance = lookat * vPosInstance;" << std::endl;
        }
        if (has_instances) {
            sstr << "    vPosInstance += instancePosition.xyz;" << std::endl;
        }
        if (reorient_uv0 || has_diffusivity || has_nontrivial_specularity || has_fresnel_exponent || fragments_depend_on_normal || (!reflectance.all_equal(0.f) && !reflect_only_y)) {
            if (has_rotation_quaternion) {
                sstr << "    vNormalInstance = rotate(rotationQuaternion, vNormalInstance);" << std::endl;
            } else {
                sstr << "    vNormalInstance = lookat * vNormalInstance;" << std::endl;
            }
        }
    } else if (has_instances && !has_lookat) {
        sstr << "    vPosInstance = vPosInstance + instancePosition;" << std::endl;
    }
    for (const auto& [t, i] : uv_map) {
        if (any(t.uv_source & BlendMapUvSource::ANY_HORIZONTAL) || (nbillboard_ids == 0)) {
            sstr << "    tex_coord" << i << " = " << tex_coords(t) << ";" << std::endl;
        } else {
            sstr << "    tex_coord" << i << " = " << tex_coords(t)
                 << " * uv_scale[billboard_id] + uv_offset[billboard_id];" << std::endl;
        }
    }
    sstr << "    gl_Position = MVP * vec4(vPosInstance, 1.0);" << std::endl;
    sstr << "    color = vCol;" << std::endl;
    if ((nbillboard_ids != 0) && !orthographic) {
        sstr << "    {" << std::endl;
        sstr << "        float dist = distance(viewPos, vPosInstance);" << std::endl;
        sstr << "        vec4 ads = alpha_distances[billboard_id];" << std::endl;
        sstr << "        if ((dist < ads[0]) || (dist > ads[3])) {" << std::endl;
        sstr << "            alpha_fac_v = 0.0;" << std::endl;
        sstr << "        } else if (dist < ads[1]) {" << std::endl;
        sstr << "            alpha_fac_v = (dist - ads[0]) / (ads[1] - ads[0]);" << std::endl;
        sstr << "        } else if (dist > ads[2]) {" << std::endl;
        sstr << "            alpha_fac_v = (ads[3] - dist) / (ads[3] - ads[2]);" << std::endl;
        sstr << "        } else {" << std::endl;
        sstr << "            alpha_fac_v = 1.0;" << std::endl;
        sstr << "        }" << std::endl;
        sstr << "    }" << std::endl;
        if (has_alpha) {
            sstr << "    alpha_fac_v *= vAlpha;" << std::endl;
        }
    } else if (has_alpha) {
        sstr << "    alpha_fac_v = vAlpha;" << std::endl;
    }
    if (has_uv_offset_u) {
        for (size_t i = 0; i < nuv_indices; ++i) {
            sstr << "    tex_coord" << i << ".s += uv_offset_u;" << std::endl;
        }
    }
    if (has_dirtmap) {
        sstr << "    vec4 pos4_dirtmap = MVP_dirtmap * vec4(vPosInstance, 1.0);" << std::endl;
        sstr << "    tex_coord_dirtmap = (pos4_dirtmap.xy / pos4_dirtmap.w + 1.0) / 2.0;" << std::endl;
        sstr << "    tex_coord_dirt = tex_coord0 * " << dirt_scale << ';' << std::endl;
    }
    if (reorient_uv0 || reorient_normals || has_nontrivial_specularity || ((fragments_depend_on_distance || has_fresnel_exponent) && !orthographic) || has_interiormap || has_horizontal_detailmap || has_reflection_map) {
        sstr << "    FragPos = vPosInstance;" << std::endl;
    }
    if (reorient_uv0 || has_diffusivity || has_nontrivial_specularity || has_fresnel_exponent || has_interiormap || fragments_depend_on_normal || (!reflectance.all_equal(0.f) && !reflect_only_y)) {
        sstr << "    Normal = vNormalInstance;" << std::endl;
    }
    if (has_normalmap || has_interiormap) {
        sstr << "    tangent = vTangent;" << std::endl;
        sstr << "    bitangent = cross(Normal, tangent);" << std::endl;
    }
    for (size_t i : lightmap_indices) {
        if (i > lights.size()) {
            THROW_OR_ABORT("Light index too large");
        }
        const auto& light = *lights[i].second;
        if (!light.vp.has_value()) {
            THROW_OR_ABORT("Light has no projection matrix");
        }
        if (!has_lightmap_depth && VisibilityCheck{*light.vp}.orthographic()) {
            sstr << "    {" << std::endl;
            sstr << "        vec4 FragPosLightSpace = MVP_light" << i << " * vec4(vPosInstance, 1.0);" << std::endl;
            sstr << "        vec2 proj_coords11 = FragPosLightSpace.xy / FragPosLightSpace.w;" << std::endl;
            sstr << "        proj_coords01_light" << i << " = proj_coords11 * 0.5 + 0.5;" << std::endl;
            sstr << "    }" << std::endl;
        } else {
            sstr << "    FragPosLightSpace" << i << " = MVP_light" << i << " * vec4(vPosInstance, 1.0);" << std::endl;
        }
    }
    for (const auto& [i, skidmark]: enumerate(skidmarks)) {
        const auto& s = *skidmark.second;
        if (!VisibilityCheck{s.vp}.orthographic()) {
            THROW_OR_ABORT("Skidmark projection matrix not orthographic");
        }
        sstr << "    {" << std::endl;
        sstr << "        vec4 FragPosSkidmarkSpace = MVP_skidmarks[" << i << "] * vec4(vPosInstance, 1.0);" << std::endl;
        sstr << "        vec2 proj_coords11 = FragPosSkidmarkSpace.xy / FragPosSkidmarkSpace.w;" << std::endl;
        sstr << "        proj_coords01_skidmarks[" << i << "] = proj_coords11 * 0.5 + 0.5;" << std::endl;
        sstr << "    }" << std::endl;
    }
    sstr << "}" << std::endl;
    if (getenv_default_bool("PRINT_SHADERS", false)) {
        linfo();
        linfo();
        linfo();
        linfo() << "Vertex";
        if (!textures_color.empty()) {
            linfo() << "Color: " + *textures_color[0]->texture_descriptor.color.filename;
        }
        std::string line;
        for (size_t i = 1; std::getline(sstr, line); ++i) {
            linfo() << i << ": " << line;
        }
    }
    return sstr.str();
}};

static void bisect_texture_layer(
    std::ostream& sstr,
    size_t left,
    size_t right,
    const std::vector<float>& continuous_layer_x,
    const std::vector<float>& continuous_layer_y,
    size_t ncalls)
{
    if (continuous_layer_x.size() != continuous_layer_y.size()) {
        THROW_OR_ABORT("Incompatible texture layer sizes");
    }
    if (left >= right) {
        THROW_OR_ABORT("Invalid interpolation bounds");
    }
    if (left >= continuous_layer_x.size()) {
        THROW_OR_ABORT("Invalid left boundary");
    }
    if (right >= continuous_layer_x.size()) {
        THROW_OR_ABORT("Invalid right boundary");
    }
    std::string indent(4 * (ncalls + 1), ' ');
    if (left + 1 == right) {
        auto len = continuous_layer_x[right] - continuous_layer_x[left];
        sstr << indent << "float layer_alpha = (texture_layer_fs - " << continuous_layer_x[left] << ") * " << (1.f / len) << ';' << std::endl;
        sstr << indent << "texture_layer_fs_transformed = mix(" << continuous_layer_y[left] << ", " << continuous_layer_y[right] << ", layer_alpha);" << std::endl;
    } else {
        auto center = (left + right) / 2;
        sstr << indent << "if (texture_layer_fs < " << continuous_layer_x[center] << ") {" << std::endl;
        bisect_texture_layer(sstr, left, center, continuous_layer_x, continuous_layer_y, ncalls + 1);
        sstr << indent << "} else {" << std::endl;
        bisect_texture_layer(sstr, center, right, continuous_layer_x, continuous_layer_y, ncalls + 1);
        sstr << indent << "}" << std::endl;
    }
}

static GenShaderText fragment_shader_text_textured_rgb_gen{[](
    const NotSortedArray<std::vector<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<Light>>>>& lights,
    const NotSortedArray<std::vector<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<Skidmark>>>>& skidmarks,
    const NotSortedArray<std::vector<BlendMapTextureAndId>>& textures_color,
    const NotSortedArray<std::vector<BlendMapTextureAndId>>& textures_alpha,
    const NotSortedArray<std::vector<size_t>>& lightmap_indices,
    const NotSortedArray<std::vector<size_t>>& light_noshadow_indices,
    const NotSortedArray<std::vector<size_t>>& light_shadow_indices,
    const NotSortedArray<std::vector<size_t>>& black_shadow_indices,
    const NotSortedStruct<AttributeIndices>& attr_ids,
    const NotSortedUvMap& uv_map,
    size_t nuv_indices,
    size_t ncweights,
    bool has_alpha,
    const std::vector<float>& continuous_layer_x,
    const std::vector<float>& continuous_layer_y,
    size_t texture_modifiers_hash,
    size_t lights_hash,
    size_t skidmarks_hash,
    size_t ntextures_color,
    size_t ntextures_normal,
    size_t ntextures_alpha,
    bool has_lightmap_color,
    bool has_lightmap_depth,
    bool has_specularmap,
    bool has_normalmap,
    bool has_reflection_map,
    bool has_dynamic_emissive,
    size_t nbillboard_ids,
    const OrderableFixedArray<float, 3>& reflectance,
    bool reflect_only_y,
    bool has_dirtmap,
    TextureLayerProperties texture_layer_properties,
    InteriorTextureSet interior_texture_set,
    const OrderableFixedArray<float, 2>& facade_inner_size,
    const OrderableFixedArray<float, 3>& interior_size,
    bool has_horizontal_detailmap,
    ColorMode dirt_color_mode,
    const OrderableFixedArray<float, 3>& emissive,
    const OrderableFixedArray<float, 3>& ambient,
    const OrderableFixedArray<float, 3>& diffuse,
    const OrderableFixedArray<float, 3>& specular,
    float specular_exponent,
    const OrderableFixedArray<float, 3>& fresnel_emissive,
    const FresnelReflectance& fresnel,
    float alpha,
    float alpha_threshold,
    const OrderableFixedArray<float, 4>& alpha_distances,
    const OrderableFixedArray<float, 2>& fog_distances,
    const OrderableFixedArray<float, 3>& fog_emissive,
    ExternalRenderPassType render_pass,
    bool reorient_normals,
    bool reorient_uv0,
    bool orthographic,
    bool fragments_depend_on_distance,
    bool fragments_depend_on_normal,
    float dirtmap_offset,
    float dirtmap_discreteness)
{
    if (nuv_indices > attr_ids->uv_count) {
        THROW_OR_ABORT("UV index too large");
    }
    if (ncweights > attr_ids->cweight_count) {
        THROW_OR_ABORT("CWeight index too large");
    }
    // Mipmapping does not work unless all textures are actually sampled everywhere.
    bool compute_interiormap_at_end = true;
    if (std::isnan(alpha_threshold)) {
        THROW_OR_ABORT("alpha_threshold is NAN => unknown blend mode");
    }
    std::string tex_coord = reorient_uv0
        ? "tex_coord_flipped"
        : "tex_coord";
    auto tex_coords = [&](const BlendMapTexture& t) {
        std::stringstream sstr;
        sstr << tex_coord << uv_map[t];
        return sstr.str();
    };
    auto sample_color = [&](size_t i)
    {
        const auto& t = textures_color.at(i);
        assert_true(t.id_color != SIZE_MAX);
        assert_true(t.tex_color != nullptr);
        if (any(texture_layer_properties)) {
            if (t->texture_descriptor.color.mipmap_mode == MipmapMode::WITH_MIPMAPS_2D) {
                return "array_texture_blend_color" + std::to_string(i) + "(texture_layer_fs_transformed)";
            } else {
                return
                    "texture(textures_color[" + std::to_string(t.id_color) +
                    "], vec3(" + tex_coords(*t) + ", texture_layer_fs_transformed))";
            }
        } else {
            return "texture(textures_color[" + std::to_string(t.id_color) + "], " + tex_coords(*t) + ')';
        }
    };
    auto sample_specularmap = [&]()
    {
        if (textures_color.size() != 1) {
            THROW_OR_ABORT("Specular maps not supported for blended textures");
        }
        const auto& t = textures_color.at(0);
        assert_true(t.id_specular == 0);
        assert_true(t.tex_specular != nullptr);
        return "texture(texture_specularmap, " + tex_coords(*t) + ')';
    };
    auto sample_normalmap_ = [&](size_t i)
    {
        const auto& t = textures_color.at(i);
        assert_true(t.id_normal != SIZE_MAX);
        assert_true(t.tex_normal != nullptr);
        if (any(texture_layer_properties)) {
            if (t->texture_descriptor.normal.mipmap_mode == MipmapMode::WITH_MIPMAPS_2D) {
                return "array_texture_blend_normal" + std::to_string(i) + "(texture_layer_fs_transformed)";
            } else {
                return
                    "texture(texture_normalmap[" + std::to_string(t.id_normal) +
                    "], vec3(" + tex_coords(*t) + ", texture_layer_fs_transformed))";
            }
        } else {
            return "texture(texture_normalmap[" + std::to_string(t.id_normal) + "], " + tex_coords(*t) + ')';
        }
    };
    auto normalmap_coords = [&](size_t i) {
        const auto& t = textures_color.at(i);
        assert_true(t.id_normal != SIZE_MAX);
        assert_true(t.tex_normal != nullptr);
        std::stringstream sstr;
        if (any(t->texture_descriptor.normal.color_mode & ColorMode::AGR_NORMAL)) {
            sstr << "(2.0 * " << sample_normalmap_(i) << ".agr - 1.0)";
        } else {
            sstr << "(2.0 * " << sample_normalmap_(i) << ".rgb - 1.0)";
        }
        return sstr.str();
    };
    auto sample_alpha = [&](size_t i) {
        const auto& t = textures_alpha.at(i);
        assert_true(t.id_color != SIZE_MAX);
        assert_true(t.tex_color != nullptr);
        if (any(texture_layer_properties)) {
            if (t->texture_descriptor.color.mipmap_mode == MipmapMode::WITH_MIPMAPS_2D) {
                return "array_texture_blend_alpha" + std::to_string(i) + "(texture_layer_fs_transformed)";
            } else {
                return
                    "texture(textures_alpha[" + std::to_string(t.id_color) +
                    "], vec3(" + tex_coords(*t) + ", texture_layer_fs_transformed))";
            }
        } else {
            return "texture(textures_alpha[" + std::to_string(t.id_color) + "], " + tex_coords(*t) + ')';
        }
    };
    std::stringstream sstr;
    sstr << std::scientific;
    sstr << SHADER_VER << FRAGMENT_PRECISION;
    sstr << "in vec3 color;" << std::endl;
    if (has_dynamic_emissive) {
        sstr << "uniform vec3 dynamic_emissive;" << std::endl;
    }
    if (!textures_color.empty()) {
        for (size_t i = 0; i < uv_map.size(); ++i) {
            sstr << "in vec2 tex_coord" << i << ";" << std::endl;
        }
    }
    for (size_t i = 0; i < ncweights; ++i) {
        sstr << "in float cweight" << i << ";" << std::endl;
    }
    sstr << "out vec4 frag_color;" << std::endl;
    if (has_alpha || ((nbillboard_ids != 0) && !orthographic)) {
        sstr << "in float alpha_fac_v;" << std::endl;
    }
    if (!reflectance.all_equal(0.f) || any(interior_texture_set & InteriorTextureSet::ANY_SPECULAR)) {
        sstr << "uniform mat3 R;" << std::endl;
    }
    if (!textures_color.empty()) {
        auto cont = any(texture_layer_properties & TextureLayerProperties::CONTINUOUS);
        auto disc = any(texture_layer_properties & TextureLayerProperties::DISCRETE);
        auto mip2 = (textures_color[0]->texture_descriptor.color.mipmap_mode == MipmapMode::WITH_MIPMAPS_2D);
        if (cont || disc) {
            for (const auto& t : textures_color) {
                auto cmip2 = (t->texture_descriptor.color.mipmap_mode == MipmapMode::WITH_MIPMAPS_2D);
                if (cmip2 != mip2) {
                    THROW_OR_ABORT("Unsupported mipmap modes in texture \"" + *t->texture_descriptor.color.filename + '"');
                }
            }
        } else if (mip2) {
            THROW_OR_ABORT(
                "Color: 2D mipmaps require a texture layer: \"" +
                *textures_color[0]->texture_descriptor.color.filename + '"');
        }
        const char* sampler_type = (cont && !mip2)
            ? "sampler3D"
            : (disc || mip2)
                ? "sampler2DArray"
                : "sampler2D";
        sstr << "uniform lowp " << sampler_type << " textures_color[" << ntextures_color << "];" << std::endl;
    }
    if (!textures_alpha.empty()) {
        auto cont = any(texture_layer_properties & TextureLayerProperties::CONTINUOUS);
        auto disc = any(texture_layer_properties & TextureLayerProperties::DISCRETE);
        auto mip2 = (textures_alpha[0]->texture_descriptor.color.mipmap_mode == MipmapMode::WITH_MIPMAPS_2D);
        if (cont || disc) {
            for (const auto& t : textures_alpha) {
                auto cmip2 = (t->texture_descriptor.color.mipmap_mode == MipmapMode::WITH_MIPMAPS_2D);
                if (cmip2 != mip2) {
                    THROW_OR_ABORT("Unsupported mipmap modes in texture \"" + *t->texture_descriptor.color.filename + '"');
                }
            }
        } else if (mip2) {
            THROW_OR_ABORT(
                "Alpha: 2D mipmaps require a texture layer: \"" +
                *textures_alpha[0]->texture_descriptor.color.filename + '"');
        }
        const char* sampler_type = (cont && !mip2)
            ? "sampler3D"
            : (disc || mip2)
                ? "sampler2DArray"
                : "sampler2D";
        sstr << "uniform lowp " << sampler_type << " textures_alpha[" << ntextures_alpha << "];" << std::endl;
    }
    if (has_lightmap_color || has_lightmap_depth) {
        if (lights.empty()) {
            THROW_OR_ABORT("No lights despite has_lightmap_color or has_lightmap_depth");
        }
        for (size_t i : lightmap_indices) {
            if (i > lights.size()) {
                THROW_OR_ABORT("Light index too large");
            }
            const auto& light = *lights[i].second;
            if (!light.vp.has_value()) {
                THROW_OR_ABORT("Light has no projection matrix");
            }
            if (!has_lightmap_depth && VisibilityCheck{ *light.vp }.orthographic()) {
                sstr << "in vec2 proj_coords01_light" << i << ";" << std::endl;
            } else {
                sstr << "in vec4 FragPosLightSpace" << i << ";" << std::endl;
            }
        }
    }
    if (!skidmarks.empty()) {
        sstr << "in vec2 proj_coords01_skidmarks[" << skidmarks.size() << "];" << std::endl;
    }
    assert_true(!(has_lightmap_color && has_lightmap_depth));
    if (has_lightmap_color) {
        for (size_t i : lightmap_indices) {
            sstr << "uniform sampler2D texture_light_color" << i << ";" << std::endl;
        }
    }
    if (has_lightmap_depth) {
        for (size_t i : lightmap_indices) {
            sstr << "uniform sampler2D texture_light_depth" << i << ";" << std::endl;
        }
    }
    if (!skidmarks.empty()) {
        sstr << "uniform sampler2D texture_skidmarks[" << skidmarks.size() << "];" << std::endl;
    }
    if (has_normalmap || any(interior_texture_set)) {
        sstr << "in vec3 tangent;" << std::endl;
        sstr << "in vec3 bitangent;" << std::endl;
    }
    if (has_normalmap) {
        auto cont = any(texture_layer_properties & TextureLayerProperties::CONTINUOUS);
        auto disc = any(texture_layer_properties & TextureLayerProperties::DISCRETE);
        auto mip2 = (textures_color[0]->texture_descriptor.normal.mipmap_mode == MipmapMode::WITH_MIPMAPS_2D);
        if (cont || disc) {
            for (const auto& t : textures_color) {
                auto cmip2 = (t->texture_descriptor.normal.mipmap_mode == MipmapMode::WITH_MIPMAPS_2D);
                if (cmip2 != mip2) {
                    THROW_OR_ABORT("Unsupported mipmap modes in texture \"" + *t->texture_descriptor.normal.filename + '"');
                }
            }
        } else if (mip2) {
            THROW_OR_ABORT(
                "Normalmap: 2D mipmaps require a texture layer: \"" +
                *textures_color[0]->texture_descriptor.normal.filename + '"');
        }
        const char* sampler_type = (cont && !mip2)
            ? "sampler3D"
            : (disc || mip2)
                ? "sampler2DArray"
                : "sampler2D";
        sstr << "uniform lowp " << sampler_type << " texture_normalmap[" << ntextures_normal << "];" << std::endl;
    }
    if (has_reflection_map) {
        sstr << "uniform samplerCube texture_reflection;" << std::endl;
    }
    if (has_dirtmap) {
        sstr << "in vec2 tex_coord_dirtmap;" << std::endl;
        sstr << "in vec2 tex_coord_dirt;" << std::endl;
        sstr << "uniform sampler2D texture_dirtmap;" << std::endl;
        sstr << "uniform sampler2D texture_dirt;" << std::endl;
    }
    if (any(texture_layer_properties & (TextureLayerProperties::ATLAS | TextureLayerProperties::VERTEX))) {
        if (any(texture_layer_properties & TextureLayerProperties::CONTINUOUS)) {
            sstr << "in float texture_layer_fs;" << std::endl;
        }
        if (any(texture_layer_properties & TextureLayerProperties::DISCRETE)) {
            sstr << "flat in lowp uint texture_layer_fs;" << std::endl;
        }
    }
    if (any(texture_layer_properties & TextureLayerProperties::UNIFORM)) {
        if (any(texture_layer_properties & TextureLayerProperties::CONTINUOUS)) {
            sstr << "uniform float texture_layer_fs;" << std::endl;
        }
        if (any(texture_layer_properties & TextureLayerProperties::DISCRETE)) {
            sstr << "uniform lowp uint texture_layer_fs;" << std::endl;
        }
    }
    if (any(interior_texture_set)) {
        sstr << "in vec3 interior_bottom_left_fs;" << std::endl;
        sstr << "in vec4 interior_uvmap_fs;" << std::endl;
        sstr << "uniform sampler2D texture_interior[" << size(interior_texture_set) << "];" << std::endl;
    }
    if (has_specularmap) {
        sstr << "uniform sampler2D texture_specularmap;" << std::endl;
    }
    if (!diffuse.all_equal(0) ||
        (!specular.all_equal(0) && specular_exponent != 0.f) ||
        (fresnel.exponent != 0.f) ||
        (!reflectance.all_equal(0.f) && !reflect_only_y) ||
        any(interior_texture_set) ||
        fragments_depend_on_normal)
    {
        sstr << "in vec3 Normal;" << std::endl;
    }
    if (!diffuse.all_equal(0) ||
        (!specular.all_equal(0) && specular_exponent != 0.f))
    {
        // sstr << "uniform vec3 lightPos;" << std::endl;
        sstr << "uniform vec3 lightDir[" << lights.size() << "];" << std::endl;
    }
    if (!ambient.all_equal(0)) {
        sstr << "uniform vec3 lightAmbient[" << lights.size() << "];" << std::endl;
    }
    if (!diffuse.all_equal(0)) {
        sstr << "uniform vec3 lightDiffuse[" << lights.size() << "];" << std::endl;
    }
    if (!specular.all_equal(0)) {
        sstr << "uniform vec3 lightSpecular[" << lights.size() << "];" << std::endl;
    }
    {
        bool pred0 = (!specular.all_equal(0) && (specular_exponent != 0.f)) || (fragments_depend_on_distance && !orthographic);
        bool pred1 = (fresnel.exponent != 0.f);
        bool pred2 = (fog_distances != default_step_distances);
        bool pred3 = !reflectance.all_equal(0.f) || any(interior_texture_set & InteriorTextureSet::ANY_SPECULAR);
        if (pred0 || pred1 || pred2 || pred3 || reorient_uv0 || any(interior_texture_set) || has_horizontal_detailmap || reorient_normals) {
            sstr << "in highp vec3 FragPos;" << std::endl;
            if ((pred0 || pred1 || pred2 || reorient_uv0 || reorient_normals) && orthographic) {
                sstr << "uniform vec3 viewDir;" << std::endl;
            }
            if (((pred0 || pred1 || pred3) && !orthographic) || any(interior_texture_set) || pred2) {
                sstr << "uniform highp vec3 viewPos;" << std::endl;
            }
        }
    }
    if (!textures_color.empty()) {
        auto mip2_color = (textures_color[0]->texture_descriptor.color.mipmap_mode == MipmapMode::WITH_MIPMAPS_2D);
        auto mip2_normal = (textures_color[0]->texture_descriptor.normal.mipmap_mode == MipmapMode::WITH_MIPMAPS_2D);
        if (mip2_color) {
            for (const auto& [i, t] : enumerate(textures_color)) {
                if (t->texture_descriptor.color.filename->empty()) {
                    continue;
                }
                sstr << "vec4 array_texture_blend_color" << i << "(in float z) {" << std::endl;
                sstr << "    float layers = " << t.tex_color->layers() << ".0;" << std::endl;
                sstr << "    float frac = fract(z * (layers - 1.0));" << std::endl;
                sstr << "    float fz = floor(z * (layers - 1.0));" << std::endl;
                sstr << "    vec4 top = texture(textures_color[" << t.id_color << "], vec3(" << tex_coord << uv_map[*t] << ", fz));" << std::endl;
                sstr << "    vec4 bottom = texture(textures_color[" << t.id_color << "], vec3(" << tex_coord << uv_map[*t] << ", fz + 1.0));" << std::endl;
                sstr << "    return mix(top, bottom, frac);" << std::endl;
                sstr << "}" << std::endl;
            }
        }
        if (mip2_normal) {
            for (const auto& [i, t] : enumerate(textures_color)) {
                if (t->texture_descriptor.normal.filename->empty()) {
                    continue;
                }
                sstr << "vec4 array_texture_blend_normal" << i << "(in float z) {" << std::endl;
                sstr << "    float layers = " << t.tex_normal->layers() << ".0;" << std::endl;
                sstr << "    float frac = fract(z * (layers - 1.0));" << std::endl;
                sstr << "    float fz = floor(z * (layers - 1.0));" << std::endl;
                sstr << "    vec4 top = texture(texture_normalmap[" << t.id_normal << "], vec3(" << tex_coord << uv_map[*t] << ", fz));" << std::endl;
                sstr << "    vec4 bottom = texture(texture_normalmap[" << t.id_normal << "], vec3(" << tex_coord << uv_map[*t] << ", fz + 1.0));" << std::endl;
                sstr << "    return mix(top, bottom, frac);" << std::endl;
                sstr << "}" << std::endl;
            }
        }
    }
    if (!textures_alpha.empty()) {
        auto mip2_alpha = (textures_alpha[0]->texture_descriptor.color.mipmap_mode == MipmapMode::WITH_MIPMAPS_2D);
        if (mip2_alpha) {
            for (const auto& [i, t] : enumerate(textures_alpha)) {
                if (t->texture_descriptor.color.filename->empty()) {
                    continue;
                }
                sstr << "vec4 array_texture_blend_alpha" << i << "(in float z) {" << std::endl;
                sstr << "    float layers = " << t.tex_color->layers() << ".0;" << std::endl;
                sstr << "    float frac = fract(z * (layers - 1.0));" << std::endl;
                sstr << "    float fz = floor(z * (layers - 1.0));" << std::endl;
                sstr << "    vec4 top = texture(textures_alpha[" << t.id_color << "], vec3(" << tex_coord << uv_map[*t] << ", fz));" << std::endl;
                sstr << "    vec4 bottom = texture(textures_alpha[" << t.id_color << "], vec3(" << tex_coord << uv_map[*t] << ", fz + 1.0));" << std::endl;
                sstr << "    return mix(top, bottom, frac);" << std::endl;
                sstr << "}" << std::endl;
            }
        }
    }
    if (!lights.empty()) {
        if (!ambient.all_equal(0)) {
            sstr << "vec3 phong_ambient(in int i) {" << std::endl;
            sstr << "    vec3 fragAmbient = vec3(" << ambient(0) << ", " << ambient(1) << ", " << ambient(2) << ");" << std::endl;
            sstr << "    return fragAmbient * lightAmbient[i];" << std::endl;
            sstr << "}" << std::endl;
        }
        if (!diffuse.all_equal(0)) {
            sstr << "vec3 phong_diffuse(in int i, in vec3 norm) {" << std::endl;
            sstr << "    vec3 fragDiffuse = vec3(" << diffuse(0) << ", " << diffuse(1) << ", " << diffuse(2) << ");" << std::endl;
            sstr << "    float diff = max(dot(norm, lightDir[i]), 0.0);" << std::endl;
            sstr << "    return fragDiffuse * diff * lightDiffuse[i];" << std::endl;
            sstr << "}" << std::endl;
        }
        if (!specular.all_equal(0) && (specular_exponent != 0.f)) {
            sstr << "vec3 phong_specular(in int i, in vec3 norm) {" << std::endl;
            if (!orthographic) {
                sstr << "    vec3 viewDir = normalize(viewPos - FragPos);" << std::endl;
            }
            sstr << "    vec3 reflectDir = reflect(-lightDir[i], norm);  " << std::endl;
            sstr << "    float spec = pow(max(dot(viewDir, reflectDir), 0.0), " << specular_exponent << ");" << std::endl;
            sstr << "    return spec * lightSpecular[i];" << std::endl;
            sstr << "}" << std::endl;
        }
    }
    if (any(interior_texture_set)) {
        sstr << "bool is_in_interior(mat3 TBN, float alpha_fac) {" << std::endl;
        if (!orthographic) {
            sstr << "    vec3 viewDir = normalize(viewPos - FragPos);" << std::endl;
        }
        sstr << "    if ((interior_uvmap_fs.y == 0.0) || (interior_uvmap_fs.w == 0.0)) {" << std::endl;
        sstr << "        return false;" << std::endl;
        sstr << "    }" << std::endl;
        sstr << "    vec3 rel_view_pos = transpose(TBN) * (viewPos - interior_bottom_left_fs);" << std::endl;
        sstr << "    vec2 rel_frag_pos = (transpose(TBN) * (FragPos - interior_bottom_left_fs)).xy;" << std::endl;
        sstr << "    rel_view_pos.x = interior_uvmap_fs.x + interior_uvmap_fs.y * rel_view_pos.x;" << std::endl;
        sstr << "    rel_frag_pos.x = interior_uvmap_fs.x + interior_uvmap_fs.y * rel_frag_pos.x;" << std::endl;
        sstr << "    rel_view_pos.y = interior_uvmap_fs.z + interior_uvmap_fs.w * rel_view_pos.y;" << std::endl;
        sstr << "    rel_frag_pos.y = interior_uvmap_fs.z + interior_uvmap_fs.w * rel_frag_pos.y;" << std::endl;
        sstr << "    vec3 rel_view_dir = vec3(rel_frag_pos, 0) - rel_view_pos;" << std::endl;
        sstr << "    float best_alpha = 1.0 / 0.0;" << std::endl;
        sstr << "    int best_axis;" << std::endl;
        sstr << "    bool best_sign;" << std::endl;
        sstr << "    vec2 best_uv;" << std::endl;
        sstr << "    vec2 facade_inner_size = vec2(" <<
            facade_inner_size(0) << ", " <<
            facade_inner_size(1) << ");" << std::endl;
        sstr << "    vec3 interior_size = vec3(" <<
            interior_size(0) << ", " <<
            interior_size(1) << ", " <<
            interior_size(2) << ");" << std::endl;
        sstr << "    vec2 w = interior_size.xy + facade_inner_size;" << std::endl;
        sstr << "    vec2 bottom = floor(rel_frag_pos / w) * w;" << std::endl;
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
            sstr << "        if (rel_view_dir[" << axis << "] < 0.0) {" << std::endl;
            sstr << "            float alpha = (bottom[" << axis << "] - rel_view_pos[" << axis << "]) / rel_view_dir[" << axis << "];" << std::endl;
            sstr << "            if (alpha < best_alpha) {" << std::endl;
            sstr << "                best_alpha = alpha;" << std::endl;
            sstr << "                best_axis = " << axis << ';' << std::endl;
            sstr << "                best_sign = false;" << std::endl;
            sstr << "                best_uv = ((rel_view_pos + alpha * rel_view_dir - vec3(bottom, 0)) / interior_size)." << axis0 << axis1 << ';' << std::endl;
            sstr << "                best_uv[" << axis << "] = -best_uv[" << axis << "];" << std::endl;
            sstr << "            }" << std::endl;
            sstr << "        } else {" << std::endl;
            sstr << "            float alpha = (interior_size[" << axis << "] + bottom[" << axis << "] - rel_view_pos[" << axis << "]) / rel_view_dir[" << axis << "];" << std::endl;
            sstr << "            if (alpha < best_alpha) {" << std::endl;
            sstr << "                best_alpha = alpha;" << std::endl;
            sstr << "                best_axis = " << axis << ';' << std::endl;
            sstr << "                best_sign = true;" << std::endl;
            sstr << "                best_uv = ((rel_view_pos + alpha * rel_view_dir - vec3(bottom, 0)) / interior_size)." << axis0 << axis1 << ';' << std::endl;
            sstr << "                best_uv[" << axis << "] = 1.0 + best_uv[" << axis << "];" << std::endl;
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
#ifdef __ANDROID__
        sstr << "    int idx = 2 * best_axis + int(best_sign);";
        for (size_t i = 0; i < size(InteriorTextureSet::INTERIOR_COLORS); ++i) {
            sstr << "    if (idx == " << i << ") frag_color = texture(texture_interior[" << i << "], best_uv);" << std::endl;
        }
#else
        sstr << "    frag_color = texture(texture_interior[2 * best_axis + int(best_sign)], best_uv);" << std::endl;
#endif
        if (any(interior_texture_set & InteriorTextureSet::BACK_SPECULAR)) {
            if (!has_reflection_map) {
                THROW_OR_ABORT("Back specular texture requires reflection map");
            }
            size_t i = index(interior_texture_set, InteriorTextureSet::BACK_SPECULAR);
            sstr << "    if (best_axis == 2) {" << std::endl;
            sstr << "        vec3 frag_specular = texture(texture_interior[" << i << "], best_uv).rgb;" << std::endl;
            sstr << "        vec3 reflectedDir = R * reflect(-viewDir, TBN[2]);" << std::endl;
            sstr << "        vec3 reflectedColor = texture(texture_reflection, vec3(reflectedDir.xy, -reflectedDir.z)).rgb;" << std::endl;
            sstr << "        frag_color.rgb += reflectedColor * frag_specular;" << std::endl;
            sstr << "    }" << std::endl;
        }
        if (any(interior_texture_set & InteriorTextureSet::FRONT_SPECULAR)) {
            if (!has_reflection_map) {
                THROW_OR_ABORT("Front specular texture requires reflection map");
            }
            size_t s = index(interior_texture_set, InteriorTextureSet::FRONT_SPECULAR);
            sstr << "    {" << std::endl;
            sstr << "        vec2 uv = (rel_frag_pos - bottom) / interior_size.xy;" << std::endl;
            sstr << "        vec3 frag_specular = texture(texture_interior[" << s << "], uv).rgb;" << std::endl;
            sstr << "        vec3 reflectedDir = R * reflect(-viewDir, TBN[2]);" << std::endl;
            sstr << "        vec3 reflectedColor = texture(texture_reflection, vec3(reflectedDir.xy, -reflectedDir.z)).rgb;" << std::endl;
            sstr << "        frag_color.rgb += reflectedColor * frag_specular;" << std::endl;
            sstr << "    }" << std::endl;
        }
        sstr << "    frag_color.a *= alpha_fac;" << std::endl;
        sstr << "    return true;" << std::endl;
        sstr << "}" << std::endl;
    }
    auto compute_normal_and_reorient_uv0 = [&](){
        if (!diffuse.all_equal(0) ||
            (!specular.all_equal(0) && (specular_exponent != 0.f)) ||
            (fresnel.exponent != 0.f) ||
            any(interior_texture_set) ||
            fragments_depend_on_normal ||
            (!reflectance.all_equal(0.f) && !reflect_only_y))
        {
            // sstr << "    vec3 norm = normalize(Normal);" << std::endl;
            sstr << "    vec3 norm = normalize(Normal);" << std::endl;
            // sstr << "    vec3 lightDir = normalize(lightPos - FragPos);" << std::endl;
        }
        if (reorient_uv0 || reorient_normals) {
            if (reorient_uv0) {
                for (size_t i = 0; i < nuv_indices; ++i) {
                    sstr << "    vec2 tex_coord_flipped" << i << " = tex_coord" << i << ";" << std::endl;
                }
            }
            if (orthographic) {
                sstr << "    if (dot(norm, viewDir) < 0.0) {" << std::endl;
            } else {
                // From: https://stackoverflow.com/questions/2523439/ipad-glsl-from-within-a-fragment-shader-how-do-i-get-the-surface-not-vertex
                sstr << "    vec3 normalvector = cross(dFdx(FragPos), dFdy(FragPos));" << std::endl;
                sstr << "    if (dot(norm, normalvector) < 0.0) {" << std::endl;
            }
            if (reorient_normals) {
                sstr << "        norm = -norm;" << std::endl;
            }
            if (reorient_uv0) {
                for (size_t i = 0; i < uv_map.size(); ++i) {
                    sstr << "        tex_coord_flipped" << i << ".s = -tex_coord" << i << ".s;" << std::endl;
                }
            }
            sstr << "    }" << std::endl;
        }
    };
    auto compute_TBN = [&](){
        sstr << "    vec3 tang = normalize(tangent);" << std::endl;
        sstr << "    vec3 bitan = normalize(bitangent);" << std::endl;
        sstr << "    mat3 TBN = mat3(tang, bitan, norm);" << std::endl;
    };
    sstr << "void main()" << std::endl;
    sstr << "{" << std::endl;
    if (has_alpha || ((nbillboard_ids != 0) && !orthographic)) {
        sstr << "    float alpha_fac = alpha_fac_v;" << std::endl;
    } else {
        sstr << "    float alpha_fac = 1.0;" << std::endl;
    }
    auto define_dist_if_necessary = [&, defined=false]() mutable {
        if (!defined) {
            if (orthographic) {
                // THROW_OR_ABORT("Distances not supported by orthographic projection");
                sstr << "    float dist = dot(viewDir, FragPos - viewPos);" << std::endl;
            } else {
                sstr << "    float dist = distance(viewPos, FragPos);" << std::endl;
            }
            defined = true;
        }
        };
    if (alpha_distances != default_linear_distances) {
        define_dist_if_necessary();
    }
    if (alpha_distances(0) != 0) {
        sstr << "    if (dist < " << alpha_distances(0) << ") {" << std::endl;
        sstr << "        discard;" << std::endl;
        sstr << "    }" << std::endl;
    }
    if (alpha_distances(3) != INFINITY) {
        sstr << "    if (dist > " << alpha_distances(3) << ") {" << std::endl;
        sstr << "        discard;" << std::endl;
        sstr << "    }" << std::endl;
    }
    if (alpha_distances(0) != alpha_distances(1)) {
        sstr << "    if (dist < " << alpha_distances(1) << ") {" << std::endl;
        sstr << "        alpha_fac = (dist - " << alpha_distances(0) << ") / " << (alpha_distances(1) - alpha_distances(0)) << ';' << std::endl;
        sstr << "    }" << std::endl;
    }
    if (alpha_distances(3) != alpha_distances(2)) {
        sstr << "    if (dist > " << alpha_distances(2) << ") {" << std::endl;
        sstr << "        alpha_fac = (" << alpha_distances(3) << " - dist) / " << (alpha_distances(3) - alpha_distances(2)) << ';' << std::endl;
        sstr << "    }" << std::endl;
    }
    if (alpha != 1.f) {
        sstr << "    alpha_fac *= " << alpha << ';' << std::endl;
    }
    if (any(interior_texture_set)) {
        compute_normal_and_reorient_uv0();
        compute_TBN();
        if (!compute_interiormap_at_end) {
            sstr << "    if (is_in_interior(TBN, alpha_fac)) {" << std::endl;
            sstr << "        return;" << std::endl;
            sstr << "    }" << std::endl;
        }
    }
    if (!any(interior_texture_set)) {
        compute_normal_and_reorient_uv0();
    }
    if (continuous_layer_x.size() != continuous_layer_y.size()) {
        THROW_OR_ABORT("Incompatible texture layer sizes");
    }
    if (!continuous_layer_x.empty()) {
        sstr << "    float texture_layer_fs_transformed;" << std::endl;
        bisect_texture_layer(sstr, 0, continuous_layer_x.size() - 1, continuous_layer_x, continuous_layer_y, 0);
    } else if (any(texture_layer_properties & TextureLayerProperties::CONTINUOUS)) {
        sstr << "    float texture_layer_fs_transformed = texture_layer_fs;" << std::endl;
    } else if (any(texture_layer_properties & TextureLayerProperties::DISCRETE)) {
        sstr << "    lowp uint texture_layer_fs_transformed = texture_layer_fs;" << std::endl;
    }
    if (textures_color.size() == 1) {
        sstr << "    vec4 texture_color_ambient_diffuse = " << sample_color(0) << ';' << std::endl;
        if (textures_alpha.empty()) {
            sstr << "    texture_color_ambient_diffuse.a *= alpha_fac;" << std::endl;
        }
    } else if (textures_color.size() > 1) {
        if (textures_color[0]->role == BlendMapRole::SUMMAND) {
            sstr << "    vec4 texture_color_ambient_diffuse = vec4(0.0, 0.0, 0.0, " << sample_color(0) << ".a);" << std::endl;
        } else if (textures_color[0]->role == BlendMapRole::DETAIL_BASE) {
            sstr << "    vec4 texture_color_ambient_diffuse = " << sample_color(0) << ';' << std::endl;
            sstr << "    vec3 sum_of_details = vec3(0.0, 0.0, 0.0);" << std::endl;
        } else if (any(textures_color[0]->role & BlendMapRole::ANY_DETAIL_MASK)) {
            sstr << "    vec4 texture_color_ambient_diffuse = vec4(0.0, 0.0, 0.0, 0.0);" << std::endl;
            sstr << "    vec3 sum_of_details = vec3(0.0, 0.0, 0.0);" << std::endl;
        } else {
            THROW_OR_ABORT("Unsupported base blend map role (0)");
        }
    }
    if (alpha_threshold != 0) {
        if (textures_color.empty()) {
            THROW_OR_ABORT("Alpha threshold requires texture");
        }
        sstr << "    if (texture_color_ambient_diffuse.a < " << alpha_threshold << ") {" << std::endl;
        sstr << "        discard;" << std::endl;
        sstr << "    }" << std::endl;
    }
    sstr << "    float weight;" << std::endl;
    auto sample_textures = [&](const std::vector<BlendMapTextureAndId>& textures, ReductionTarget target) {
        sstr << "    weight = 1.0;" << std::endl;
        if (target == ReductionTarget::COLOR) {
            sstr << "    float sum_weights = 0.0;" << std::endl;
            if (has_normalmap) {
                if (textures_color[0]->texture_descriptor.normal.filename->empty()) {
                    sstr << "    vec3 tnorm = vec3(0.0, 0.0, 1.0);" << std::endl;
                } else if (textures_color[0]->role != BlendMapRole::DETAIL_BASE) {
                    sstr << "    vec3 tnorm = vec3(0.0, 0.0, 0.0);" << std::endl;
                } else {
                    sstr << "    vec3 tnorm = " << normalmap_coords(0) << ';' << std::endl;
                }
            }
        }
        if (target == ReductionTarget::ALPHA) {
            if (has_alpha) {
                sstr << "    texture_color_ambient_diffuse.a = clamp(alpha_fac, 0.0, 1.0);" << std::endl;
            } else {
                sstr << "    texture_color_ambient_diffuse.a = 0.0;" << std::endl;
            }
        }
        if (alpha_distances == default_linear_distances) {
            if (fog_distances != default_step_distances) {
                define_dist_if_necessary();
            } else {
                for (const BlendMapTextureAndId& t : textures) {
                    if (t->distances != default_linear_distances) {
                        define_dist_if_necessary();
                        break;
                    }
                }
            }
        }
        for (const auto& [i, t] : enumerate(textures)) {
            if ((i == 0) && (t->role == BlendMapRole::DETAIL_BASE)) {
                continue;
            }
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
            if (t->weight == 0.f) {
                checks.emplace_back("(weight != 0.0)");
            }
            if (!checks.empty()) {
                sstr << "        if (" << join(" && ", checks) << ") {" << std::endl;
            }
            if (t->weight != 0.f) {
                sstr << "            weight = " << t->weight << ';' << std::endl;
            }
            if (t->distances(0) != t->distances(1)) {
                sstr << "            if (dist <= " << t->distances(1) << ") {" << std::endl;
                sstr << "                weight *= (dist - " << t->distances(0) << ") / " << (t->distances(1) - t->distances(0)) << ';' << std::endl;
                sstr << "            }" << std::endl;
            }
            if (t->distances(3) != t->distances(2)) {
                sstr << "            if (dist >= " << t->distances(2) << ") {" << std::endl;
                sstr << "                weight *= (" << t->distances(3) << " - dist) / " << (t->distances(3) - t->distances(2)) << ';' << std::endl;
                sstr << "            }" << std::endl;
            }
            if (t->cosines(0) != t->cosines(1)) {
                sstr << "            if (cosine <= " << t->cosines(1) << ") {" << std::endl;
                sstr << "                weight *= (cosine - " << t->cosines(0) << ") / " << (t->cosines(1) - t->cosines(0)) << ';' << std::endl;
                sstr << "            }" << std::endl;
            }
            if (t->cosines(3) != t->cosines(2)) {
                sstr << "            if (cosine >= " << t->cosines(2) << ") {" << std::endl;
                sstr << "                weight *= (" << t->cosines(3) << " - cosine) / " << (t->cosines(3) - t->cosines(2)) << ';' << std::endl;
                sstr << "            }" << std::endl;
            }
            if (t->cweight_id != UINT32_MAX) {
                if (t->cweight_id >= ncweights) {
                    THROW_OR_ABORT("cweight index too large");
                }
                sstr << "            weight *= cweight" << t->cweight_id << ';' << std::endl;
                if (t->min_detail_weight != 0.f) {
                    sstr << "            weight = max(weight, " << t->min_detail_weight << ");" << std::endl;
                }
            }
            if (any(t->role & BlendMapRole::ANY_DETAIL_MASK)) {
                char c = [&t](){
                    switch (t->role) {
                        case BlendMapRole::DETAIL_MASK_R: return 'r';
                        case BlendMapRole::DETAIL_MASK_G: return 'g';
                        case BlendMapRole::DETAIL_MASK_B: return 'b';
                        case BlendMapRole::DETAIL_MASK_A: return 'a';
                        default: THROW_OR_ABORT("Unknown detail mask");
                    }
                }();
                sstr << "            float w = " << sample_color(i) << '.' << c << ';' << std::endl;
                if (t->reduction == BlendMapReductionOperation::TIMES) {
                    sstr << "            weight *= w;" << std::endl;
                } else if (t->reduction == BlendMapReductionOperation::FEATHER) {
                    if (t->discreteness == 0) {
                        THROW_OR_ABORT("Detail-mask with feather has zero discreteness");
                    }
                    sstr << "            weight += (0.5 - abs(weight - 0.5)) * (w + " << t->plus << ") * " << t->discreteness << ';' << std::endl;
                    sstr << "            weight = clamp(weight, 0.0, 1.0);" << std::endl;
                } else if (t->reduction == BlendMapReductionOperation::INVERT) {
                    sstr << "            weight = 1.0 - weight;" << std::endl;
                } else {
                    THROW_OR_ABORT("Unknown reduction operation");
                }
                if (t->min_detail_weight != 0.f) {
                    sstr << "            weight = max(weight, " << t->min_detail_weight << ");" << std::endl;
                }
            } else if (any(t->texture_descriptor.color.color_mode & ColorMode::RGBA) && (t->discreteness != 0)) {
                sstr << "            vec4 bcolor = " << sample_color(i) << ".rgba;" << std::endl;
                sstr << "            float final_weight = weight * clamp(0.5 + " << t->discreteness << " * (bcolor.a - 0.5), 0.0, 1.0);" << std::endl;
                // sstr << "            weight *= bcolor.a;" << std::endl;
            } else if (
                any(t->texture_descriptor.color.color_mode & ColorMode::RGB) ||
                (any(t->texture_descriptor.color.color_mode & ColorMode::RGBA) && (t->discreteness == 0)))
            {
                sstr << "            vec3 bcolor = " << sample_color(i) << ".rgb;" << std::endl;
                sstr << "            float final_weight = weight;" << std::endl;
            } else if (target == ReductionTarget::ALPHA) {
                if (t->texture_descriptor.color.color_mode != ColorMode::GRAYSCALE) {
                    THROW_OR_ABORT("Alpha-texture not loaded as grayscale");
                }
                sstr << "            float intensity = " << sample_alpha(i) << ".r;" << std::endl;
                sstr << "            float final_weight = weight;" << std::endl;
            } else {
                THROW_OR_ABORT("Texture: \"" + *t->texture_descriptor.color.filename + "\". Unsupported color mode: \"" + color_mode_to_string(t->texture_descriptor.color.color_mode) + '"');
            }
            if (any(t->role & BlendMapRole::ANY_DETAIL_MASK)) {
                // Do nothing
            } else {
                char rop;
                switch (t->reduction) {
                    case BlendMapReductionOperation::PLUS:     rop = '+'; break;
                    case BlendMapReductionOperation::MINUS:    rop = '-'; break;
                    case BlendMapReductionOperation::TIMES:    rop = '*'; break;
                    case BlendMapReductionOperation::FEATHER:  rop = '?'; break;
                    case BlendMapReductionOperation::BLEND:    rop = '+'; break;
                    case BlendMapReductionOperation::COLORIZE: rop = '?'; break;
                    default: THROW_OR_ABORT("Unknown blendmap reduction type");
                }
                if (t->role == BlendMapRole::DETAIL_COLOR) {
                    sstr << "            sum_of_details " << rop << "= final_weight * bcolor.rgb;" << std::endl;
                } else if (t->role == BlendMapRole::SUMMAND) {
                    if (target == ReductionTarget::COLOR) {
                        if (t->reduction == BlendMapReductionOperation::REPLACE_COLOR) {
                            sstr << "            float brightness = dot(texture_color_ambient_diffuse.rgb, vec3(0.2989, 0.5870, 0.1140));" << std::endl;
                            sstr << "            texture_color_ambient_diffuse.rgb *= (1.0 - final_weight);" << std::endl;
                            sstr << "            texture_color_ambient_diffuse.rgb += bcolor.rgb * (final_weight * brightness * " << t->discreteness << ");" << std::endl;
                        } else if (t->reduction == BlendMapReductionOperation::COLORIZE) {
                            sstr << "            texture_color_ambient_diffuse.rgb *= (1.0 - final_weight) + bcolor.rgb * (final_weight * " << t->discreteness << ");" << std::endl;
                        } else {
                            if (t->reduction == BlendMapReductionOperation::BLEND) {
                                sstr << "            texture_color_ambient_diffuse.rgb *= (1.0 - final_weight);" << std::endl;
                            }
                            sstr << "            texture_color_ambient_diffuse.rgb " << rop << "= final_weight * bcolor.rgb;" << std::endl;
                        }
                    } else if (target == ReductionTarget::ALPHA) {
                        if (t->reduction == BlendMapReductionOperation::FEATHER) {
                            sstr << "            float h = (0.5 - abs(texture_color_ambient_diffuse.a - 0.5)) * 2.0;" << std::endl;
                            sstr << "            h = clamp(h, 0.0, 1.0);" << std::endl;
                            sstr << "            float intens = clamp((intensity + " << + t->plus << ") * " << t->discreteness << ", 0.0, 1.0);" << std::endl;
                            sstr << "            texture_color_ambient_diffuse.a = mix(texture_color_ambient_diffuse.a, intens, h);" << std::endl;
                        } else {
                            sstr << "            texture_color_ambient_diffuse.a " << rop << "= final_weight * intensity + " << t->plus << ";" << std::endl;
                        }
                    } else {
                        THROW_OR_ABORT("Unknown reduction target (1)");
                    }
                } else if (!any(t->role & BlendMapRole::ANY_DETAIL_MASK)) {
                    THROW_OR_ABORT("Unknown blend map role");
                }
                if (target == ReductionTarget::COLOR) {
                    if (has_normalmap) {
                        if (t->texture_descriptor.normal.filename->empty()) {
                            sstr << "            tnorm.z += final_weight;" << std::endl;
                        } else {
                            sstr << "            tnorm += final_weight * " << normalmap_coords(i) << ';' << std::endl;
                        }
                    }
                    sstr << "            sum_weights += final_weight;" << std::endl;
                }
            }
            if (!checks.empty()) {
                if (t->weight != 0.f) {
                    sstr << "        } else {" << std::endl;
                    sstr << "            weight = 0.0;" << std::endl;
                }
                sstr << "        }" << std::endl;
            }
            sstr << "    }" << std::endl;
        }
        if ((target == ReductionTarget::ALPHA) && (textures[0]->reweight_mode != BlendMapReweightMode::UNDEFINED)) {
            THROW_OR_ABORT("Target is alpha and reweighting is not undefined in texture \"" + *textures[0]->texture_descriptor.color.filename + '"');
        }
        if ((target == ReductionTarget::COLOR) && (textures[0]->reweight_mode == BlendMapReweightMode::UNDEFINED)) {
            THROW_OR_ABORT("Target is color and reweighting is undefined in texture \"" + *textures[0]->texture_descriptor.color.filename + '"');
        }
        if ((target == ReductionTarget::COLOR) && (textures[0]->reweight_mode == BlendMapReweightMode::ENABLED)) {
            sstr << "    if (sum_weights < 1e-3) {" << std::endl;
            sstr << "        texture_color_ambient_diffuse.rgb = vec3(1.0, 0.0, 1.0);" << std::endl;
            sstr << "    } else {" << std::endl;
            sstr << "        texture_color_ambient_diffuse.rgb /= sum_weights;" << std::endl;
        } else {
            sstr << "    {" << std::endl;
        }
        if ((textures[0]->role == BlendMapRole::SUMMAND) ||
            any(textures[0]->role & BlendMapRole::ANY_DETAIL_MASK))
        {
            // Do nothing
        } else if (textures[0]->role == BlendMapRole::DETAIL_BASE) {
            sstr << "        texture_color_ambient_diffuse.rgb *= " << textures[0]->weight << " * sum_of_details;" << std::endl;
        } else {
            THROW_OR_ABORT("Unsupported base blend map role (1)");
        }
        sstr << "    }" << std::endl;
    };
    if (textures_color.size() > 1) {
        sample_textures(textures_color, ReductionTarget::COLOR);
        // sstr << "    texture_color_ambient_diffuse.rgb /= max(1e-6, sum_weights);" << std::endl;
    }
    if (!textures_alpha.empty()) {
        sample_textures(textures_alpha, ReductionTarget::ALPHA);
    }
    if (has_normalmap) {
        if (textures_color.size() == 1) {
            sstr << "    vec3 tnorm = " << normalmap_coords(0) << ';' << std::endl;
        }
        if (!any(interior_texture_set)) {
            compute_TBN();
        }
        sstr << "    norm = normalize(TBN * tnorm);" << std::endl;
    }
    sstr << "    vec3 frag_brightness_emissive_ambient_diffuse = vec3(0.0, 0.0, 0.0);" << std::endl;
    sstr << "    vec3 frag_brightness_specular = vec3(0.0, 0.0, 0.0);" << std::endl;
    auto compute_light_color = [&sstr, &lights](size_t i){
        if (i > lights.size()) {
            THROW_OR_ABORT("Light index too large");
        }
        const auto& light = *lights[i].second;
        if (!light.vp.has_value()) {
            THROW_OR_ABORT("Light has no projection matrix");
        }
        if (VisibilityCheck{*light.vp}.orthographic()) {
            sstr << "            vec3 light_color = texture(texture_light_color" << i << ", proj_coords01_light" << i << ").rgb;" << std::endl;
        } else {
            sstr << "            vec2 proj_coords11 = FragPosLightSpace" << i << ".xy / FragPosLightSpace" << i << ".w;" << std::endl;
            sstr << "            vec2 proj_coords01 = proj_coords11 * 0.5 + 0.5;" << std::endl;
            sstr << "            vec3 light_color = texture(texture_light_color" << i << ", proj_coords01).rgb;" << std::endl;
        }
    };
    if (has_lightmap_color && !black_shadow_indices.empty()) {
        sstr << "    vec3 black_fac = vec3(1.0, 1.0, 1.0);" << std::endl;
    }
    assert_true(!(has_lightmap_color && has_lightmap_depth));
    if (has_lightmap_color && !black_shadow_indices.empty()) {
        sstr << "    {" << std::endl;
        for (size_t i : black_shadow_indices) {
            assert_true(i < lights.size());
            sstr << "        {" << std::endl;
            compute_light_color(i);
            sstr << "            black_fac = min(black_fac, light_color);" << std::endl;
            sstr << "        }" << std::endl;
        }
        sstr << "    }" << std::endl;
    }
    if (!skidmarks.empty()) {
        sstr << "    vec3 skidmark_fac = vec3(1.0, 1.0, 1.0);" << std::endl;
        sstr << "    vec3 water_wave_fac = vec3(0.0, 0.0, 0.0);" << std::endl;
        sstr << "    vec3 sea_spray_color = vec3(0.0, 0.0, 0.0);" << std::endl;
        size_t nwater_waves = 0;
        for (const auto& [i, s]: enumerate(skidmarks)) {
            [&](){
                switch (s.second->particle_type) {
                case ParticleType::NONE:
                    THROW_OR_ABORT("Particle type \"none\" does not require a skidmark");
                case ParticleType::SMOKE:
                    THROW_OR_ABORT("Smoke does not require a skidmark");
                case ParticleType::SKIDMARK:
                    sstr << "    skidmark_fac = min(skidmark_fac, texture(texture_skidmarks[" << i << "], proj_coords01_skidmarks[" << i << "]).rgb);" << std::endl;
                    return;
                case ParticleType::WATER_WAVE:
                    sstr << "    water_wave_fac += texture(texture_skidmarks[" << i << "], proj_coords01_skidmarks[" << i << "]).rgb;" << std::endl;
                    ++nwater_waves;
                    return;
                case ParticleType::SEA_SPRAY:
                    sstr << "    sea_spray_color = max(sea_spray_color, texture(texture_skidmarks[" << i << "], proj_coords01_skidmarks[" << i << "]).rgb);" << std::endl;
                    return;
                }
                THROW_OR_ABORT("Unknown particle type");
            }();
        }
        if (nwater_waves == 0) {
            sstr << "    water_wave_fac = vec3(1.0, 1.0, 1.0);" << std::endl;
        } else {
            sstr << "    water_wave_fac *= 2.0 / " << nwater_waves << ".0;" << std::endl;
        }
    }
    if (has_lightmap_depth && !light_shadow_indices.empty()) {
        for (size_t i : light_shadow_indices) {
            assert_true(i < lights.size());
            sstr << "    {" << std::endl;
            sstr << "        vec3 proj_coords11 = FragPosLightSpace" << i << ".xyz / FragPosLightSpace" << i << ".w;" << std::endl;
            sstr << "        vec3 proj_coords01 = proj_coords11 * 0.5 + 0.5;" << std::endl;
            sstr << "        if (proj_coords01.z - 0.00002 < texture(texture_light_depth" << i << ", proj_coords01.xy).r) {" << std::endl;
            if (!ambient.all_equal(0) && any(lights[i].second->ambient != 0.f)) {
                sstr << "            frag_brightness_emissive_ambient_diffuse += phong_ambient(" << i << ");" << std::endl;
            }
            if (!diffuse.all_equal(0) && any(lights[i].second->diffuse != 0.f)) {
                sstr << "            frag_brightness_emissive_ambient_diffuse += phong_diffuse(" << i << ", norm);" << std::endl;
            }
            if (!specular.all_equal(0) && any(lights[i].second->specular != 0.f)) {
                if (specular_exponent == 0.f) {
                    sstr << "            frag_brightness_specular += lightSpecular[" << i << "];" << std::endl;
                } else {
                    sstr << "            frag_brightness_specular += phong_specular(" << i << ", norm);" << std::endl;
                }
            }
            sstr << "        }" << std::endl;
            sstr << "    }" << std::endl;
        }
    }
    if (!has_lightmap_depth && !light_shadow_indices.empty()) {
        for (size_t i : light_shadow_indices) {
            assert_true(i < lights.size());
            sstr << "    {" << std::endl;
            if (has_lightmap_color) {
                compute_light_color(i);
            } else {
                sstr << "        vec3 light_color = vec3(1.0, 1.0, 1.0);" << std::endl;
            }
            if (!ambient.all_equal(0) && any(lights[i].second->ambient != 0.f)) {
                sstr << "        frag_brightness_emissive_ambient_diffuse += light_color * phong_ambient(" << i << ");" << std::endl;
            }
            if (!diffuse.all_equal(0) && any(lights[i].second->diffuse != 0.f)) {
                sstr << "        frag_brightness_emissive_ambient_diffuse += light_color * phong_diffuse(" << i << ", norm);" << std::endl;
            }
            if (!specular.all_equal(0) && any(lights[i].second->specular != 0.f)) {
                if (specular_exponent == 0.f) {
                    sstr << "        frag_brightness_specular += light_color * lightSpecular[" << i << "];" << std::endl;
                } else {
                    sstr << "        frag_brightness_specular += light_color * phong_specular(" << i << ", norm);" << std::endl;
                }
            }
            sstr << "    }" << std::endl;
        }
    }
    if (!light_noshadow_indices.empty()) {
        for (size_t i : light_noshadow_indices) {
            assert_true(i < lights.size());
            sstr << "    {" << std::endl;
            if (!ambient.all_equal(0) && any(lights[i].second->ambient != 0.f)) {
                sstr << "        frag_brightness_emissive_ambient_diffuse += phong_ambient(" << i << ");" << std::endl;
            }
            if (!diffuse.all_equal(0) && any(lights[i].second->diffuse != 0.f)) {
                sstr << "        frag_brightness_emissive_ambient_diffuse += phong_diffuse(" << i << ", norm);" << std::endl;
            }
            if (!specular.all_equal(0) && any(lights[i].second->specular != 0.f)) {
                if (specular_exponent == 0.f) {
                    sstr << "        frag_brightness_specular += lightSpecular[" << i << "];" << std::endl;
                } else {
                    sstr << "        frag_brightness_specular += phong_specular(" << i << ", norm);" << std::endl;
                }
            }
            sstr << "    }" << std::endl;
        }
    }
    sstr << "    frag_brightness_specular *= vec3(" << specular(0) << ", " << specular(1) << ", " << specular(2) << ");" << std::endl;
    if (has_specularmap) {
        sstr << "    vec3 frag_specular = " << sample_specularmap() << ".rgb;" << std::endl;
    } else {
        sstr << "    float frag_specular = 1.0;" << std::endl;
    }
    if (!reflectance.all_equal(0.f) || (fresnel.exponent != 0.f)) {
        if (!orthographic) {
            sstr << "    vec3 viewDir = normalize(viewPos - FragPos);" << std::endl;
        }
    }
    if (fresnel.exponent != 0.f) {
        sstr << "    {" << std::endl;
        // Note that normalmaps can generate opposing normals (which the abs(...) kind of deals with).
        sstr << "        float fresnel_factor0 = pow(max(1.0 - abs(dot(viewDir, norm)), 0.0), " << fresnel.exponent << ");" << std::endl;
        sstr << "        float fresnel_factor = " << fresnel.min << " + " << (fresnel.max - fresnel.min) << " * fresnel_factor0;" << std::endl;
        sstr << "        frag_specular *= fresnel_factor;" << std::endl;
        sstr << "    }" << std::endl;
    }
    if (!reflectance.all_equal(0.f)) {
        if (reflect_only_y) {
            sstr << "    vec3 reflectedDir = R * reflect(-viewDir, R[1]);" << std::endl;
        } else {
            sstr << "    vec3 reflectedDir = R * reflect(-viewDir, norm);" << std::endl;
        }
        // Modification proposed in https://learnopengl.com/Advanced-OpenGL/Cubemaps#comment-5197766106
        // This works in combination with not flipping the y-coordinate when loading the texture.
        sstr << "    vec3 reflectance = vec3(" << reflectance(0) << ", " << reflectance(1) << ", " << reflectance(2) << ");" << std::endl;
        sstr << "    frag_brightness_specular += reflectance * texture(texture_reflection, vec3(reflectedDir.xy, -reflectedDir.z)).rgb;" << std::endl;
    }
    if (!fresnel_emissive.all_equal(0.f)) {
        sstr << "    vec3 fresnel_emissive = vec3(" << fresnel_emissive(0) << ", " << fresnel_emissive(1) << ", " << fresnel_emissive(2) << ");" << std::endl;
        sstr << "    frag_brightness_specular += fresnel_emissive;" << std::endl;
    }
    if (!ambient.all_equal(0) || !diffuse.all_equal(0) || !specular.all_equal(0)) {
        if (has_lightmap_color && !black_shadow_indices.empty()) {
            sstr << "    frag_brightness_emissive_ambient_diffuse *= black_fac;" << std::endl;
            sstr << "    frag_brightness_specular *= black_fac;" << std::endl;
        }
    }
    if (!emissive.all_equal(0.f)) {
        sstr << "    frag_brightness_emissive_ambient_diffuse += vec3(" << emissive(0) << ", " << emissive(1) << ", " << emissive(2) << ");" << std::endl;
    }
    if (has_dynamic_emissive) {
        sstr << "    frag_brightness_emissive_ambient_diffuse += dynamic_emissive;" << std::endl;
    }
    if (!skidmarks.empty()) {
        sstr << "    frag_brightness_emissive_ambient_diffuse *= skidmark_fac * water_wave_fac;" << std::endl;
        sstr << "    frag_brightness_specular *= skidmark_fac * water_wave_fac;" << std::endl;
    }
    if (textures_color.empty() && has_dirtmap) {
        THROW_OR_ABORT("Combination of ((ntextures_color == 0) && has_dirtmap) is not supported");
    }
    if (has_dirtmap) {
        sstr << "    float dirtiness = texture(texture_dirtmap, tex_coord_dirtmap).r;" << std::endl;
        sstr << "    vec4 dirt_color = texture(texture_dirt, tex_coord_dirt);" << std::endl;
        if (dirt_color_mode == ColorMode::RGBA) {
            sstr << "    dirtiness *= dirt_color.a;" << std::endl;
        } else if (dirt_color_mode != ColorMode::RGB) {
            THROW_OR_ABORT("Unsupported dirt color mode: " + color_mode_to_string(dirt_color_mode));
        }
        sstr << "    dirtiness += " << dirtmap_offset << ';' << std::endl;
        sstr << "    dirtiness = clamp(0.5 + " << dirtmap_discreteness << " * (dirtiness - 0.5), 0.0, 1.0);" << std::endl;
        // sstr << "    dirtiness += clamp(0.005 + 80 * (0.98 - norm.y), 0.0, 1.0);" << std::endl;
        sstr << "    frag_color.a = texture_color_ambient_diffuse.a;" << std::endl;
        sstr << "    frag_color.rgb = texture_color_ambient_diffuse.rgb * (1.0 - dirtiness)" << std::endl;
        sstr << "                     + dirt_color.rgb * dirtiness;" << std::endl;
        sstr << "    frag_color.rgb *= color;" << std::endl;
        sstr << "    frag_brightness_specular.rgb *= (1.0 - dirtiness);" << std::endl;
    } else if (!textures_color.empty()) {
        sstr << "    frag_color = texture_color_ambient_diffuse * vec4(color, 1.0);" << std::endl;
    } else {
        sstr << "    frag_color = vec4(color, alpha_fac);" << std::endl;
    }
    if (!skidmarks.empty()) {
        sstr << "    frag_color.rgb += 0.4 * sea_spray_color;" << std::endl;
    }
    sstr << "    frag_color.rgb *= frag_brightness_emissive_ambient_diffuse;" << std::endl;
    if ((fresnel.exponent != 0.f) || has_specularmap) {
        sstr << "    frag_color.rgb = mix(frag_color.rgb, frag_brightness_specular, frag_specular);" << std::endl;
    } else {
        sstr << "    frag_color.rgb += frag_brightness_specular;" << std::endl;
    }
    if (any(render_pass & ExternalRenderPassType::LIGHTMAP_BLOBS_MASK)) {
        // Do nothing (keep colors)
    } else if (any(render_pass & ExternalRenderPassType::LIGHTMAP_COLOR_MASK)) {
        sstr << "    frag_color.r = 0.5;" << std::endl;
        sstr << "    frag_color.g = 0.5;" << std::endl;
        sstr << "    frag_color.b = 0.5;" << std::endl;
    } else if (fog_distances != default_step_distances) {
        define_dist_if_necessary();
        sstr << "    {" << std::endl;
        sstr << "        vec3 fog_emissive = vec3(" << fog_emissive(0) << ", " << fog_emissive(1) << ", " << fog_emissive(2) << ");" << std::endl;
        sstr << "        float t = clamp((dist - " << fog_distances(0) << ") / " << (fog_distances(1) - fog_distances(0)) << ", 0.0, 1.0);" << std::endl;
        sstr << "        frag_color.rgb = mix(frag_color.rgb, fog_emissive, t);" << std::endl;
        sstr << "    }" << std::endl;
    }
    if (any(interior_texture_set) && compute_interiormap_at_end) {
        sstr << "    is_in_interior(TBN, alpha_fac);" << std::endl;
    }
    sstr << "}" << std::endl;
    if (getenv_default_bool("PRINT_SHADERS", false)) {
        linfo();
        linfo();
        linfo();
        linfo() << "Fragment";
        if (!textures_color.empty()) {
            linfo() << "Color: " + *textures_color[0]->texture_descriptor.color.filename;
        }
        std::string line;
        for (size_t i = 1; std::getline(sstr, line); ++i) {
            linfo() << i << ": " << line;
        }
    }
    return sstr.str();
}};

ColoredVertexArrayResource::ColoredVertexArrayResource(
    std::shared_ptr<AnimatedColoredVertexArrays> triangles,
    Vertices&& vertices,
    std::unique_ptr<Instances>&& instances,
    std::weak_ptr<ColoredVertexArrayResource> vertex_data)
    : triangles_res_{ std::move(triangles) }
    , scene_node_resources_{ RenderingContextStack::primary_scene_node_resources() }
    , rendering_resources_{ RenderingContextStack::primary_rendering_resources() }
    , vertex_arrays_{ std::move(vertices) }
    , instances_{ std::move(instances) }
    , vertex_data_{ std::move(vertex_data) }
{
#ifdef DEBUG
    triangles_res_->check_consistency();
#endif
}

ColoredVertexArrayResource::ColoredVertexArrayResource(
    const std::list<std::shared_ptr<ColoredVertexArray<float>>>& striangles,
    const std::list<std::shared_ptr<ColoredVertexArray<CompressedScenePos>>>& dtriangles,
    Vertices&& vertices,
    std::unique_ptr<Instances>&& instances,
    std::weak_ptr<ColoredVertexArrayResource> vertex_data)
: ColoredVertexArrayResource{
    std::make_shared<AnimatedColoredVertexArrays>(),
    std::move(vertices),
    std::move(instances),
    std::move(vertex_data)}
{
    triangles_res_->scvas = striangles;
    triangles_res_->dcvas = dtriangles;
#ifdef DEBUG
    triangles_res_->check_consistency();
#endif
}

ColoredVertexArrayResource::ColoredVertexArrayResource(
    const std::shared_ptr<ColoredVertexArray<float>>& striangles,
    Vertices&& vertices,
    std::unique_ptr<Instances>&& instances,
    std::weak_ptr<ColoredVertexArrayResource> vertex_data)
    : ColoredVertexArrayResource(
        std::list<std::shared_ptr<ColoredVertexArray<float>>>{striangles},
        std::list<std::shared_ptr<ColoredVertexArray<CompressedScenePos>>>{},
        std::move(vertices),
        std::move(instances),
        std::move(vertex_data))
{}

ColoredVertexArrayResource::ColoredVertexArrayResource(
    const std::shared_ptr<ColoredVertexArray<CompressedScenePos>>& dtriangles,
    std::unique_ptr<Instances>&& instances)
    : ColoredVertexArrayResource(
        std::list<std::shared_ptr<ColoredVertexArray<float>>>{},
        std::list<std::shared_ptr<ColoredVertexArray<CompressedScenePos>>>{dtriangles},
        Vertices{},
        std::move(instances),
        std::weak_ptr<ColoredVertexArrayResource>())
{}

// From: https://stackoverflow.com/questions/26379311/calling-initializer-list-constructor-via-make-unique-make-shared
template<typename T>
static std::initializer_list<T> make_init_list(std::initializer_list<T>&& l) {
    return l;
}

ColoredVertexArrayResource::ColoredVertexArrayResource(
    const std::shared_ptr<ColoredVertexArray<float>>& striangles,
    const std::shared_ptr<IInstanceBuffers>& instances)
    : ColoredVertexArrayResource(
        striangles,
        Vertices{},
        std::make_unique<Instances>(make_init_list({ Instances::value_type{striangles.get(), instances} })),
        std::weak_ptr<ColoredVertexArrayResource>())
{}

ColoredVertexArrayResource::ColoredVertexArrayResource(
    const std::shared_ptr<ColoredVertexArray<float>>& striangles,
    const std::shared_ptr<IVertexData>& vertices)
    : ColoredVertexArrayResource(
        striangles,
        Vertices(make_init_list({ Vertices::value_type{striangles.get(), vertices} })),
        nullptr,
        std::weak_ptr<ColoredVertexArrayResource>())
{}

ColoredVertexArrayResource::ColoredVertexArrayResource(const std::shared_ptr<AnimatedColoredVertexArrays>& triangles)
    : ColoredVertexArrayResource(
        triangles,
        Vertices{},
        nullptr,
        std::weak_ptr<ColoredVertexArrayResource>())
{}

ColoredVertexArrayResource::ColoredVertexArrayResource(
    const std::list<std::shared_ptr<ColoredVertexArray<float>>>& striangles)
    : ColoredVertexArrayResource{ striangles, std::list<std::shared_ptr<ColoredVertexArray<CompressedScenePos>>>{} }
{}

ColoredVertexArrayResource::ColoredVertexArrayResource(
    const std::list<std::shared_ptr<ColoredVertexArray<CompressedScenePos>>>& dtriangles)
    : ColoredVertexArrayResource{ std::list<std::shared_ptr<ColoredVertexArray<float>>>{}, dtriangles }
{}

ColoredVertexArrayResource::ColoredVertexArrayResource(
    const std::list<std::shared_ptr<ColoredVertexArray<float>>>& striangles,
    const std::list<std::shared_ptr<ColoredVertexArray<CompressedScenePos>>>& dtriangles)
    : ColoredVertexArrayResource(
        striangles,
        dtriangles,
        Vertices{},
        nullptr,
        std::weak_ptr<ColoredVertexArrayResource>())
{}

ColoredVertexArrayResource::ColoredVertexArrayResource(
    const std::shared_ptr<ColoredVertexArray<float>>& striangles)
    : ColoredVertexArrayResource(
        { striangles },
        std::list<std::shared_ptr<ColoredVertexArray<CompressedScenePos>>>{},
        Vertices{},
        nullptr,
        std::weak_ptr<ColoredVertexArrayResource>())
{}

ColoredVertexArrayResource::ColoredVertexArrayResource(
    const std::shared_ptr<ColoredVertexArray<CompressedScenePos>>& dtriangles)
    : ColoredVertexArrayResource(
        std::list<std::shared_ptr<ColoredVertexArray<float>>>{},
        { dtriangles },
        Vertices{},
        nullptr,
        std::weak_ptr<ColoredVertexArrayResource>())
{}

ColoredVertexArrayResource::~ColoredVertexArrayResource() = default;

void ColoredVertexArrayResource::preload(const RenderableResourceFilter& filter) const {
    auto preload_textures = [this, &filter](const auto& cvas) {
        for (const auto& [i, cva] : enumerate(cvas)) {
            if (!filter.matches(i, *cva)) {
                continue;
            }
            for (auto& t : cva->material.textures_color) {
                rendering_resources_.preload(t.texture_descriptor);
            }
            for (auto& t : cva->material.textures_alpha) {
                rendering_resources_.preload(t.texture_descriptor);
            }
        }
    };
    preload_textures(triangles_res_->scvas);
    preload_textures(triangles_res_->dcvas);
    if (ContextQuery::is_initialized()) {
        for (const auto &cva : triangles_res_->scvas) {
            if (requires_aggregation(*cva)) {
                continue;
            }
            get_vertex_array(cva, TaskLocation::FOREGROUND);
            if (instances_ != nullptr) {
                instances_->at(cva.get())->wait();
            }
        }
    }
}

void ColoredVertexArrayResource::instantiate_child_renderable(const ChildInstantiationOptions& options) const
{
#ifdef DEBUG
    triangles_res_->check_consistency();
#endif
    if (options.rendering_resources == nullptr) {
        THROW_OR_ABORT("ColoredVertexArrayResource::instantiate_child_renderable: rendering-resources is null");
    }
    options.scene_node->add_renderable(options.instance_name, std::make_shared<RenderableColoredVertexArray>(
        *options.rendering_resources,
        shared_from_this(),
        options.renderable_resource_filter));
}

void ColoredVertexArrayResource::instantiate_root_renderables(const RootInstantiationOptions& options) const {
    {
        std::list<std::shared_ptr<ColoredVertexArray<CompressedScenePos>>> dcvas_node_triangles;
        for (const auto& cva : triangles_res_->dcvas) {
            if (cva->material.aggregate_mode == AggregateMode::NODE_TRIANGLES) {
                dcvas_node_triangles.push_back(cva);
            }
        }
        auto split = split_meshes(
            dcvas_node_triangles,
            cluster_center_by_grid<CompressedScenePos, ScenePos>(fixed_full<ScenePos, 3>(options.triangle_cluster_width)),
            *options.instance_name + "_split");
        for (const auto& [i, c] : enumerate(cluster_meshes<CompressedScenePos>(
            split,
            cva_to_grid_center<CompressedScenePos, ScenePos>(fixed_full<ScenePos, 3>(options.triangle_cluster_width)),
            *options.instance_name + "_cluster")))
        {
            auto resource_name = VariableAndHash<std::string>{*options.instance_name + std::to_string(i)};
            auto transformed = c.cva->template translated<float>(-c.position, "_centered");
            transformed->morphology.center_distances2 = options.center_distances2;
            auto resource = std::make_shared<ColoredVertexArrayResource>(transformed);
            auto ctrafo = options.absolute_model_matrix * TransformationMatrix<SceneDir, ScenePos, 3>{
                fixed_identity_array<SceneDir, 3>(),
                c.position.template casted<ScenePos>()};
            auto node = make_unique_scene_node(
                ctrafo.t,
                matrix_2_tait_bryan_angles(ctrafo.R),
                ctrafo.get_scale(),
                PoseInterpolationMode::DISABLED);
            resource->instantiate_child_renderable(ChildInstantiationOptions{
                .rendering_resources = options.rendering_resources,
                .instance_name = VariableAndHash{ *options.instance_name + "_hri_split_arrays" },
                .scene_node = node.ref(DP_LOC),
                .renderable_resource_filter = options.renderable_resource_filter});
            auto node_name = VariableAndHash<std::string>{*options.instance_name + std::to_string(i) + "_hri_split_node"};
            options.scene.auto_add_root_node(
                node_name,
                std::move(node),
                RenderingDynamics::STATIC);
            if (options.instantiated_nodes != nullptr) {
                options.instantiated_nodes->emplace_back(std::move(node_name));
            }
        }
    }
    {
        std::list<std::shared_ptr<ColoredVertexArray<CompressedScenePos>>> dcvas_node_object;
        for (const auto& cva : triangles_res_->dcvas) {
            if (cva->material.aggregate_mode == AggregateMode::NODE_OBJECT) {
                dcvas_node_object.push_back(cva);
            }
        }
        for (const auto& [i, c] : enumerate(cluster_meshes<CompressedScenePos>(
            dcvas_node_object,
            cva_to_grid_center<CompressedScenePos, ScenePos>(fixed_full<ScenePos, 3>(options.object_cluster_width)),
            *options.instance_name + "_cluster")))
        {
            auto center = c.cva->aabb().data().center();
            auto tm = TranslationMatrix{ center.casted<ScenePos>() };
            auto trafo = options.absolute_model_matrix * tm;
            auto scva = c.cva->translated<float>(-center, "_centered");
            scva->morphology.center_distances2 += (float)options.object_cluster_width;
            auto rcva = std::make_shared<ColoredVertexArrayResource>(std::move(scva));
            rcva->instantiate_root_renderables(
                RootInstantiationOptions{
                    .rendering_resources = options.rendering_resources,
                    .imposters = options.imposters,
                    .instantiated_nodes = options.instantiated_nodes,
                    .instance_name = VariableAndHash<std::string>{ "building_cluster_" + std::to_string(i) },
                    .absolute_model_matrix = trafo,
                    .scene = options.scene,
                    .max_imposter_texture_size = options.max_imposter_texture_size,
                    .renderable_resource_filter = options.renderable_resource_filter
                });
        }
    }
    if (!triangles_res_->scvas.empty() || !triangles_res_->dcvas.empty()) {
        auto node = make_unique_scene_node(
            options.absolute_model_matrix.t,
            matrix_2_tait_bryan_angles(options.absolute_model_matrix.R),
            options.absolute_model_matrix.get_scale(),
            PoseInterpolationMode::DISABLED);
        instantiate_child_renderable(ChildInstantiationOptions{
            .rendering_resources = options.rendering_resources,
            .instance_name = options.instance_name,
            .scene_node = node.ref(DP_LOC),
            .renderable_resource_filter = options.renderable_resource_filter});
        if (options.max_imposter_texture_size != 0) {
            if (options.imposters == nullptr) {
                THROW_OR_ABORT("Imposters not set for \"" + *options.instance_name + '"');
            }
            auto imposter_node_name = *options.instance_name + "-" + std::to_string(options.scene.get_uuid());
            options.imposters->set_imposter_info(node.ref(DP_LOC), { imposter_node_name, options.max_imposter_texture_size });
        }
        auto node_name = VariableAndHash<std::string>{*options.instance_name + "_cva_world"};
        options.scene.auto_add_root_node(
            node_name,
            std::move(node),
            RenderingDynamics::STATIC);
        if (options.instantiated_nodes != nullptr) {
            options.instantiated_nodes->emplace_back(std::move(node_name));
        }
    }
}

std::shared_ptr<AnimatedColoredVertexArrays> ColoredVertexArrayResource::get_arrays(
    const ColoredVertexArrayFilter& filter) const
{
    auto result = std::make_shared<AnimatedColoredVertexArrays>();
    result->skeleton = triangles_res_->skeleton;
    result->bone_indices = triangles_res_->bone_indices;
    for (const auto& cva : triangles_res_->scvas) {
        if (filter.matches(*cva)) {
            result->scvas.push_back(cva);
        }
    }
    for (const auto& cva : triangles_res_->dcvas) {
        if (filter.matches(*cva)) {
            result->dcvas.push_back(cva);
        }
    }
    return result;
}

void ColoredVertexArrayResource::generate_triangle_rays(size_t npoints, const FixedArray<float, 3>& lengths, bool delete_triangles) {
    auto gen_triangle_rays = [&]<typename TPos>(const std::list<std::shared_ptr<ColoredVertexArray<TPos>>>& cvas)
    {
        for (auto& t : cvas) {
            auto r = Mlib::generate_triangle_rays(t->triangles, npoints, lengths.template casted<TPos>());
            t->lines.reserve(t->lines.size() + r.size());
            for (const auto& l : r) {
                t->lines.push_back({
                    ColoredVertex<TPos>{
                        l[0],
                        Colors::WHITE,
                        {0.f, 0.f}
                    },
                    ColoredVertex<TPos>{
                        l[1],
                        Colors::WHITE,
                        {0.f, 1.f}
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
        THROW_OR_ABORT("generate_ray requires exactly one triangle mesh");
    }
    auto gen_ray = [&]<typename TPos>(const std::list<std::shared_ptr<ColoredVertexArray<TPos>>>& cvas)
    {
        cvas.front()->lines.push_back({
            ColoredVertex<TPos>{
                from,
                Colors::WHITE,
                {0.f, 0.f},
                fixed_zeros<float, 3>(),
                fixed_zeros<float, 3>(),
            },
            ColoredVertex<TPos>{
                to,
                Colors::WHITE,
                {0.f, 1.f},
                fixed_zeros<float, 3>(),
                fixed_zeros<float, 3>(),
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

std::shared_ptr<ISceneNodeResource> ColoredVertexArrayResource::generate_grind_lines(
    float edge_angle,
    float averaged_normal_angle,
    const ColoredVertexArrayFilter& filter) const
{
    return std::make_shared<ColoredVertexArrayResource>(
        triangles_res_->generate_grind_lines(edge_angle, averaged_normal_angle, filter));
}

std::shared_ptr<ISceneNodeResource> ColoredVertexArrayResource::generate_contour_edges() const {
    std::list<std::shared_ptr<ColoredVertexArray<float>>> dest_scvas;
    std::list<std::shared_ptr<ColoredVertexArray<CompressedScenePos>>> dest_dcvas;
    for (auto& t : triangles_res_->scvas) {
        dest_scvas.push_back(
            t->generate_contour_edges());
    }
    for (auto& t : triangles_res_->dcvas) {
        dest_dcvas.push_back(
            t->generate_contour_edges());
    }
    return std::make_shared<ColoredVertexArrayResource>(dest_scvas, dest_dcvas);
}

void ColoredVertexArrayResource::create_barrier_triangle_hitboxes(
    float depth,
    PhysicsMaterial destination_physics_material,
    const ColoredVertexArrayFilter& filter)
{
    triangles_res_->create_barrier_triangle_hitboxes(depth, destination_physics_material, filter);
}

// std::shared_ptr<ISceneNodeResource> ColoredVertexArrayResource::extract_by_predicate(
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

// std::shared_ptr<ISceneNodeResource> ColoredVertexArrayResource::copy_by_predicate(
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
        THROW_OR_ABORT("Duplicate add/remove flags");
    }
    auto modify_tags = [&](auto& cvas){
        for (auto& cva : cvas) {
            if (filter.matches(*cva)) {
                cva->morphology.physics_material |= add;
                cva->morphology.physics_material &= ~remove;
                if (any(add & PhysicsMaterial::ATTR_CONTAINS_SKIDMARKS)) {
                    cva->material.skidmarks |= ParticleType::SKIDMARK;
                }
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

AggregateMode ColoredVertexArrayResource::get_aggregate_mode() const {
    std::set<AggregateMode> aggregate_modes;
    for (const auto& t : triangles_res_->scvas) {
        aggregate_modes.insert(t->material.aggregate_mode);
    }
    for (const auto& t : triangles_res_->dcvas) {
        aggregate_modes.insert(t->material.aggregate_mode);
    }
    if (aggregate_modes.empty()) {
        THROW_OR_ABORT("Cannot determine aggregate mode of empty array");
    }
    if (aggregate_modes.size() != 1) {
        THROW_OR_ABORT("aggregate_mode is not unique");
    }
    return *aggregate_modes.begin();
}

PhysicsMaterial ColoredVertexArrayResource::get_physics_material() const {
    auto result = PhysicsMaterial::NONE;
    for (const auto& cva : triangles_res_->scvas) {
        result |= cva->morphology.physics_material;
    }
    for (const auto& cva : triangles_res_->dcvas) {
        result |= cva->morphology.physics_material;
    }
    return result;
}

void ColoredVertexArrayResource::print(std::ostream& ostr) const {
    ostr << "ColoredVertexArrayResource\n";
    triangles_res_->print(ostr);
}

AttributeIndexCalculator ColoredVertexArrayResource::get_attribute_index_calculator(
    const ColoredVertexArray<float>& cva) const
{
    bool has_injected_texture_layers = [&]() {
        std::shared_lock lock{ mutex_ };
        auto pva = vertex_arrays_.try_get(&cva);
        if (pva == nullptr) {
            return false;
        }
        return
            (*pva)->has_continuous_triangle_texture_layers() ||
            (*pva)->has_discrete_triangle_texture_layers();
        }();
    return AttributeIndexCalculator{
        .has_position = true,
        .has_color = true,
        .has_normal =
            cva.material.reorient_uv0 ||
            !cva.material.shading.diffuse.all_equal(0) ||
            !cva.material.shading.specular.all_equal(0) ||
            (!cva.material.shading.reflectance.all_equal(0.f) && !cva.material.reflect_only_y && !cva.material.reflection_map->empty()) ||
            (cva.material.shading.fresnel.reflectance.exponent != 0.f) ||
            fragments_depend_on_normal(cva.material.textures_color) ||
            !cva.material.interior_textures.empty(),
        .has_tangent = has_normalmap(cva.material.textures_color) || !cva.material.interior_textures.empty(),
        .has_instance_attrs = instances_ != nullptr,
        .has_rotation_quaternion = (instances_ != nullptr) && (cva.material.transformation_mode == TransformationMode::ALL),
        .has_billboard_ids = !cva.material.billboard_atlas_instances.empty(),
        .has_bone_indices = !triangles_res_->bone_indices.empty(),
        .has_bone_weights = !triangles_res_->bone_indices.empty(),
        .has_texture_layer =
            ((instances_ != nullptr) && instances_->at(&cva)->has_continuous_texture_layer()) ||
            !cva.continuous_triangle_texture_layers.empty() ||
            !cva.discrete_triangle_texture_layers.empty() ||
            has_injected_texture_layers,
        .has_interior_mapping_bottom_left = !cva.material.interior_textures.empty(),
        .has_interior_mapping_multiplier = !cva.material.interior_textures.empty(),
        .nuvs = cva.uv1.size() + 1,
        .ncweights = cva.cweight.size(),
        .has_alpha = !cva.alpha.empty()
    };
}

const ColoredRenderProgram& ColoredVertexArrayResource::get_render_program(
    const RenderProgramIdentifier& id,
    const ColoredVertexArray<float>& cva,
    const std::vector<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<Light>>>& filtered_lights,
    const std::vector<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<Skidmark>>>& filtered_skidmarks,
    const std::vector<size_t>& lightmap_indices,
    const std::vector<size_t>& light_noshadow_indices,
    const std::vector<size_t>& light_shadow_indices,
    const std::vector<size_t>& black_shadow_indices,
    const std::vector<BlendMapTextureAndId>& textures_color,
    const std::vector<BlendMapTextureAndId>& textures_alpha) const
{
    auto& rps = rendering_resources_.render_programs();
    if (auto it = rps.try_get(id); it != nullptr) {
        return **it;
    }
    std::scoped_lock lock{ mutex_ };
    if (auto it = rps.try_get(id); it != nullptr) {
        return **it;
    }
    auto rp = std::make_unique<ColoredRenderProgram>();
    assert_true(triangles_res_->bone_indices.empty() == !triangles_res_->skeleton);
    auto attr_idc = get_attribute_index_calculator(cva);
    auto attr_ids = attr_idc.build();
    Map<UvMapKey, size_t> uv_map;
    for (const auto& t : textures_color) {
        NotSortedUvMap{ uv_map }.insert(*t);
    }
    for (const auto& t : textures_alpha) {
        NotSortedUvMap{ uv_map }.insert(*t);
    }
    const char* vs_text = vertex_shader_text_gen(
        NotSortedArray{ filtered_lights },
        NotSortedArray{ filtered_skidmarks },
        NotSortedArray{ textures_color },
        NotSortedArray{ textures_alpha },
        NotSortedArray{ lightmap_indices },
        NotSortedStruct{ attr_idc },
        NotSortedStruct{ attr_ids },
        NotSortedUvMap{ uv_map },
        id.nuv_indices,
        id.ncweights,
        id.has_alpha,
        id.texture_modifiers_hash,
        id.lights_hash,
        id.skidmarks_hash,
        id.nbillboard_ids,
        id.reflectance,
        id.reflect_only_y,
        !id.lightmap_indices_color.empty(),
        !id.lightmap_indices_depth.empty(),
        id.ntextures_normal != 0,
        id.ntextures_reflection != 0,
        id.ntextures_dirt != 0,
        any(id.interior_texture_set),
        id.has_horizontal_detailmap,
        !id.diffuse.all_equal(0),
        !id.specular.all_equal(0) && (id.specular_exponent != 0.f),
        (id.fresnel.exponent != 0.f),
        id.has_instances,
        id.has_lookat,
        id.has_yangle,
        id.has_rotation_quaternion,
        id.has_uv_offset_u,
        triangles_res_->bone_indices.size(),
        id.texture_layer_properties,
        id.reorient_normals,
        id.reorient_uv0,
        id.fog_distances != default_step_distances,
        id.orthographic,
        id.fragments_depend_on_distance,
        id.fragments_depend_on_normal,
        id.dirt_scale);
    const char* fs_text = fragment_shader_text_textured_rgb_gen(
        NotSortedArray{ filtered_lights },
        NotSortedArray{ filtered_skidmarks },
        NotSortedArray{ textures_color },
        NotSortedArray{ textures_alpha },
        NotSortedArray{ lightmap_indices },
        NotSortedArray{ light_noshadow_indices },
        NotSortedArray{ light_shadow_indices },
        NotSortedArray{ black_shadow_indices },
        NotSortedStruct{ attr_ids },
        NotSortedUvMap{ uv_map },
        id.nuv_indices,
        id.ncweights,
        id.has_alpha,
        id.continuous_layer_x,
        id.continuous_layer_y,
        id.texture_modifiers_hash,
        id.lights_hash,
        id.skidmarks_hash,
        id.ntextures_color,
        id.ntextures_normal,
        id.ntextures_alpha,
        !id.lightmap_indices_color.empty(),
        !id.lightmap_indices_depth.empty(),
        id.has_specularmap,
        id.ntextures_normal != 0,
        id.ntextures_reflection != 0,
        id.has_dynamic_emissive,
        id.nbillboard_ids,
        id.reflectance,
        id.reflect_only_y,
        id.ntextures_dirt != 0,
        id.texture_layer_properties,
        id.interior_texture_set,
        id.facade_inner_size,
        id.interior_size,
        id.has_horizontal_detailmap,
        id.dirt_color_mode,
        id.emissive,
        id.ambient,
        id.diffuse,
        id.specular,
        id.specular_exponent,
        id.fresnel_emissive,
        id.fresnel,
        any(id.blend_mode & BlendMode::ANY_CONTINUOUS)
            ? id.alpha
            : 1.f,
        (id.blend_mode == BlendMode::OFF) ||
        any(id.blend_mode & BlendMode::CONTINUOUS_MASK) ||
        textures_color.empty()
            ? 0.f
            : any(id.blend_mode & BlendMode::THRESHOLD_02_MASK)
                ? 0.2f
                : any(id.blend_mode & BlendMode::THRESHOLD_05_MASK)
                    ? 0.5f
                    : any(id.blend_mode & BlendMode::THRESHOLD_08_MASK)
                        ? 0.8f
                        : NAN,
        id.alpha_distances,
        id.fog_distances,
        id.fog_emissive,
        id.render_pass,
        id.reorient_normals,
        id.reorient_uv0,
        id.orthographic,
        id.fragments_depend_on_distance,
        id.fragments_depend_on_normal,
        id.dirtmap_offset,
        id.dirtmap_discreteness);
    try {
        rp->allocate(vs_text, fs_text);

        rp->mvp_location = rp->get_uniform_location("MVP");
        if (id.has_uv_offset_u) {
            rp->uv_offset_u_location = rp->get_uniform_location("uv_offset_u");
        }
        for (size_t i = 0; i < id.ntextures_color; ++i) {
            rp->texture_color_locations[i] = rp->get_uniform_location(("textures_color[" + std::to_string(i) + "]").c_str());
        }
        for (size_t i = 0; i < id.ntextures_alpha; ++i) {
            rp->texture_alpha_locations[i] = rp->get_uniform_location(("textures_alpha[" + std::to_string(i) + "]").c_str());
        }
        if (!id.lightmap_indices_color.empty() || !id.lightmap_indices_depth.empty()) {
            for (size_t i : lightmap_indices) {
                rp->mvp_light_locations[i] = rp->get_uniform_location(("MVP_light" + std::to_string(i)).c_str());
            }
        } else {
            // Do nothing
            // rp->mvp_light_location = 0;
        }
        for (size_t i = 0; i < filtered_skidmarks.size(); ++i) {
            rp->mvp_skidmarks_locations[i] = rp->get_uniform_location(("MVP_skidmarks[" + std::to_string(i) + "]").c_str());
        }
        if (any(id.texture_layer_properties & TextureLayerProperties::UNIFORM)) {
            rp->texture_layer_location_uniform = rp->get_uniform_location("texture_layer_fs");
        } else {
            rp->texture_layer_location_uniform = 0;
        }
        if (id.nbillboard_ids != 0) {
            rp->vertex_scale_location = rp->get_uniform_location("vertex_scale");
            rp->uv_scale_location = rp->get_uniform_location("uv_scale");
            rp->uv_offset_location = rp->get_uniform_location("uv_offset");
            if (any(id.texture_layer_properties & TextureLayerProperties::ATLAS)) {
                rp->texture_layers_location_atlas = rp->get_uniform_location("texture_layers");
            } else {
                rp->texture_layers_location_atlas = 0;
            }
            if (!id.orthographic) {
                rp->alpha_distances_location = rp->get_uniform_location("alpha_distances");
            } else {
                rp->alpha_distances_location = 0;
            }
        } else {
            rp->vertex_scale_location = 0;
            rp->uv_scale_location = 0;
            rp->uv_offset_location = 0;
            rp->texture_layers_location_atlas = 0;
            rp->alpha_distances_location = 0;
        }
        if (id.has_dynamic_emissive) {
            rp->dynamic_emissive_location = rp->get_uniform_location("dynamic_emissive");
        } else {
            rp->dynamic_emissive_location = 0;
        }
        assert(id.lightmap_indices_color.empty() || id.lightmap_indices_depth.empty());
        if (!id.lightmap_indices_color.empty()) {
            for (size_t i : lightmap_indices) {
                rp->texture_lightmap_color_locations[i] = rp->get_uniform_location(("texture_light_color" + std::to_string(i)).c_str());
            }
        } else {
            // Do nothing
            // rp->texture_lightmap_color_location = 0;
        }
        if (!id.lightmap_indices_depth.empty()) {
            for (size_t i : lightmap_indices) {
                rp->texture_lightmap_depth_locations[i] = rp->get_uniform_location(("texture_light_depth" + std::to_string(i)).c_str());
            }
        } else {
            // Do nothing
            // rp->texture_lightmap_depth_location = 0;
        }
        if (id.ntextures_normal != 0) {
            for (const auto& r : textures_color) {
                if (!r->texture_descriptor.normal.filename->empty()) {
                    rp->texture_normalmap_locations[r.id_normal] = rp->get_uniform_location(("texture_normalmap[" + std::to_string(r.id_normal) + "]").c_str());
                }
            }
        }
        for (size_t i = 0; i < filtered_skidmarks.size(); ++i) {
            rp->texture_skidmark_locations[i] = rp->get_uniform_location(("texture_skidmarks[" + std::to_string(i) + "]").c_str());
        }
        if (id.ntextures_reflection != 0) {
            rp->texture_reflection_location = rp->get_uniform_location("texture_reflection");
        } else {
            rp->texture_reflection_location = 0;
        }
        if (id.ntextures_dirt != 0) {
            rp->mvp_dirtmap_location = rp->get_uniform_location("MVP_dirtmap");
            rp->texture_dirtmap_location = rp->get_uniform_location("texture_dirtmap");
            rp->texture_dirt_location = rp->get_uniform_location("texture_dirt");
        } else {
            rp->mvp_dirtmap_location = 0;
            rp->texture_dirtmap_location = 0;
            rp->texture_dirt_location = 0;
        }
        if (any(id.interior_texture_set)) {
            auto n = size(id.interior_texture_set);
            for (size_t i = 0; i < n; ++i) {
                rp->texture_interiormap_location(i) = rp->get_uniform_location(("texture_interior[" + std::to_string(i) + "]").c_str());
            }
        } else {
            rp->texture_interiormap_location = 0;
        }
        if (id.has_specularmap) {
            rp->texture_specularmap_location = rp->get_uniform_location("texture_specularmap");
        } else {
            rp->texture_specularmap_location = 0;
        }
        if (!id.reflectance.all_equal(0.f) || any(id.interior_texture_set & InteriorTextureSet::ANY_SPECULAR)) {
            rp->r_location = rp->get_uniform_location("R");
        } else {
            rp->r_location = 0;
        }
        {
            bool light_dir_required = !id.diffuse.all_equal(0) || !id.specular.all_equal(0);
            if (id.reorient_uv0 || light_dir_required || (id.fragments_depend_on_distance && !id.orthographic) || id.fragments_depend_on_normal || any(id.interior_texture_set)) {
                if (light_dir_required) {
                    for (size_t i = 0; i < filtered_lights.size(); ++i) {
                        if (!any(filtered_lights.at(i).second->shadow_render_pass & ExternalRenderPassType::LIGHTMAP_IS_BLACK_MASK)) {
                            rp->light_dir_locations[i] = rp->get_uniform_location(("lightDir[" + std::to_string(i) + "]").c_str());
                        }
                    }
                }
            }
        }
        // rp->light_position_location = rp->get_uniform_location("lightPos");
        assert_true(triangles_res_->bone_indices.empty() == !triangles_res_->skeleton);
        for (size_t i = 0; i < triangles_res_->bone_indices.size(); ++i) {
            rp->pose_positions[i] = rp->get_uniform_location(("bone_positions[" + std::to_string(i) + "]").c_str());
            rp->pose_quaternions[i] = rp->get_uniform_location(("bone_quaternions[" + std::to_string(i) + "]").c_str());
        }
        for (size_t i = 0; i < filtered_lights.size(); ++i) {
            if (!id.ambient.all_equal(0) &&
                any(filtered_lights.at(i).second->ambient != 0.f) &&
                !any(filtered_lights.at(i).second->shadow_render_pass & ExternalRenderPassType::LIGHTMAP_IS_BLACK_MASK))
            {
                rp->light_ambients[i] = rp->get_uniform_location(("lightAmbient[" + std::to_string(i) + "]").c_str());
            }
            if (!id.diffuse.all_equal(0) &&
                any(filtered_lights.at(i).second->diffuse != 0.f) &&
                !any(filtered_lights.at(i).second->shadow_render_pass & ExternalRenderPassType::LIGHTMAP_IS_BLACK_MASK))
            {
                rp->light_diffuses[i] = rp->get_uniform_location(("lightDiffuse[" + std::to_string(i) + "]").c_str());
            }
            if (!id.specular.all_equal(0) &&
                any(filtered_lights.at(i).second->specular != 0.f) &&
                !any(filtered_lights.at(i).second->shadow_render_pass & ExternalRenderPassType::LIGHTMAP_IS_BLACK_MASK))
            {
                rp->light_speculars[i] = rp->get_uniform_location(("lightSpecular[" + std::to_string(i) + "]").c_str());
            }
        }
        {
            bool pred0 =
                id.has_lookat ||
                (!id.specular.all_equal(0) && (id.specular_exponent != 0.f)) ||
                !id.reflectance.all_equal(0.f) ||
                any(id.interior_texture_set & InteriorTextureSet::ANY_SPECULAR) ||
                (id.fragments_depend_on_distance && !id.orthographic) ||
                (id.fresnel.exponent != 0.f);
            bool pred1 = (id.fog_distances != default_step_distances);
            if (pred0 || pred1 || id.reorient_uv0 || id.reorient_normals || any(id.interior_texture_set)) {
                if (((pred0 || pred1 || id.reorient_uv0 || id.reorient_normals) && id.orthographic)) {
                    rp->view_dir = rp->get_uniform_location("viewDir");
                    rp->view_pos = 0;
                } else {
                    rp->view_dir = 0;
                }
                if ((pred0 && !id.orthographic) || any(id.interior_texture_set) || pred1) {
                    rp->view_pos = rp->get_uniform_location("viewPos");
                } else {
                    rp->view_pos = 0;
                }
            } else {
                rp->view_dir = 0;
                rp->view_pos = 0;
            }
        }
        if (id.has_horizontal_detailmap) {
            rp->horizontal_detailmap_remainder = rp->get_uniform_location("horizontal_detailmap_remainder");
        } else {
            rp->horizontal_detailmap_remainder = 0;
        }

        auto& result = *rp;
        rps.add(id, std::move(rp));
        return result;
    } catch (const std::runtime_error& e) {
        std::string identifier;
        if (!textures_color.empty()) {
            identifier = "\nAmbient+diffuse: " + *textures_color[0]->texture_descriptor.color.filename;
        }
        THROW_OR_ABORT(
            std::string("Could not generate render program.\n") +
            e.what() +
            identifier +
            "\nVertex shader:\n" + vs_text +
            "\nFragment shader:\n" + fs_text);
    }
}

bool ColoredVertexArrayResource::requires_aggregation(const ColoredVertexArray<float> &cva) const {
    return (cva.material.aggregate_mode != AggregateMode::NONE) && (instances_ == nullptr);
}

IVertexData& ColoredVertexArrayResource::get_vertex_array(
    const std::shared_ptr<ColoredVertexArray<float>>& cva,
    TaskLocation task_location) const
{
    if (requires_aggregation(*cva)) {
        THROW_OR_ABORT("get_vertex_array called on aggregated object \"" + cva->name.full_name() + '"');
    }
    {
        std::shared_lock lock{ mutex_ };
        if (auto* pva = vertex_arrays_.try_get(cva.get());
            (pva != nullptr) && ((*pva)->initialized()))
        {
            return **pva;
        }
    }
    std::scoped_lock lock{ mutex_ };
    auto* pva = vertex_arrays_.try_get(cva.get());
    if ((pva != nullptr) && (*pva)->initialized()) {
        return **pva;
    }
    std::unique_ptr<IVertexData> si;
    auto& va = [&]() -> IVertexData& {
        if (pva == nullptr) {
            if (cva->triangles.empty()) {
                THROW_OR_ABORT("ColoredVertexArrayResource::get_vertex_array on empty array \"" + cva->name.full_name() + '"');
            }
            std::shared_ptr<IArrayBuffer> inherited_vertices;
            if (auto p = vertex_data_.lock(); p != nullptr) {
                if (auto v = p->vertex_arrays_.try_get(cva.get()); v != nullptr) {
                    inherited_vertices = (*v)->vertex_buffer().fork();
                }
            }
            si = std::make_unique<DistantTriangleHider>(
                cva,
                cva->triangles.size(),
                inherited_vertices);
            return *si;
        } else {
            return **pva;
        }
        }();
    // https://stackoverflow.com/a/13405205/2292832
    va.initialize();
    if (va.vertex_buffer().is_awaited()) {
        va.vertex_buffer().bind();
    } else {
        va.vertex_buffer().set(cva->triangles, task_location);
    }

    auto attr_idc = get_attribute_index_calculator(*cva);
    auto attr_ids = attr_idc.build();

    ColoredVertex<float>* cv = nullptr;
    CHK(glEnableVertexAttribArray(attr_ids.idx_position));
    CHK(glVertexAttribPointer(attr_ids.idx_position, 3, GL_FLOAT, GL_FALSE, sizeof(ColoredVertex<float>), &cv->position));
    CHK(glEnableVertexAttribArray(attr_ids.idx_color));
    CHK(glVertexAttribPointer(attr_ids.idx_color, 3, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ColoredVertex<float>), &cv->color));
    CHK(glEnableVertexAttribArray(attr_ids.idx_uv_0));
    CHK(glVertexAttribPointer(attr_ids.idx_uv_0, 2, GL_FLOAT, GL_FALSE, sizeof(ColoredVertex<float>), &cv->uv));
    // The vertex array is cached by cva => Use material properties, not the RenderProgramIdentifier.
    if (attr_idc.has_normal) {
        CHK(glEnableVertexAttribArray(attr_ids.idx_normal));
        CHK(glVertexAttribPointer(attr_ids.idx_normal, 3, GL_FLOAT, GL_FALSE, sizeof(ColoredVertex<float>), &cv->normal));
    }
    // The vertex array is cached by cva => Use material properties, not the RenderProgramIdentifier.
    if (attr_idc.has_tangent) {
        CHK(glEnableVertexAttribArray(attr_ids.idx_tangent));
        CHK(glVertexAttribPointer(attr_ids.idx_tangent, 3, GL_FLOAT, GL_FALSE, sizeof(ColoredVertex<float>), &cv->tangent));
    }
    if (!cva->uv1.empty()) {
        if (cva->uv1.size() + 1 != attr_ids.uv_count) {
            THROW_OR_ABORT("Unexpected number of UV indices");
        }
        for (const auto& [i, uv1] : enumerate(cva->uv1)) {
            if (va.uv1_buffer(i).is_awaited()) {
                va.uv1_buffer(i).bind();
            } else {
                if (uv1.size() != cva->triangles.size()) {
                    THROW_OR_ABORT("#uv1 != #triangles");
                }
                va.uv1_buffer(i).set(uv1, task_location);
            }
            CHK(glEnableVertexAttribArray(integral_cast<GLuint>(attr_ids.idx_uv_1 + i)));
            CHK(glVertexAttribPointer(integral_cast<GLuint>(attr_ids.idx_uv_1 + i), 2, GL_FLOAT, GL_FALSE, sizeof(FixedArray<float, 2>), nullptr));
        }
    }
    {
        if (cva->cweight.size() != attr_ids.cweight_count) {
            THROW_OR_ABORT("Unexpected number of weight coefficients");
        }
        for (const auto& [i, cweight] : enumerate(cva->cweight)) {
            if (va.cweight_buffer(i).is_awaited()) {
                va.cweight_buffer(i).bind();
            } else {
                if (cweight.size() != cva->triangles.size()) {
                    THROW_OR_ABORT("#cweights != #triangles");
                }
                va.cweight_buffer(i).set(cweight, task_location);
            }
            CHK(glEnableVertexAttribArray(integral_cast<GLuint>(attr_ids.idx_cweight_0 + i)));
            CHK(glVertexAttribPointer(integral_cast<GLuint>(attr_ids.idx_cweight_0 + i), 1, GL_FLOAT, GL_FALSE, sizeof(float), nullptr));
        }
    }
    if (!cva->alpha.empty()) {
        if (va.alpha_buffer().is_awaited()) {
            va.alpha_buffer().bind();
        } else {
            if (cva->alpha.size() != cva->triangles.size()) {
                THROW_OR_ABORT("#alpha != #triangles");
            }
            va.alpha_buffer().set(cva->alpha, task_location);
        }
        CHK(glEnableVertexAttribArray(integral_cast<GLuint>(attr_ids.idx_alpha)));
        CHK(glVertexAttribPointer(integral_cast<GLuint>(attr_ids.idx_alpha), 1, GL_FLOAT, GL_FALSE, sizeof(float), nullptr));
    }
    if (instances_ != nullptr) {
        instances_->at(cva.get())->bind(
            attr_ids.idx_instance_attrs,
            attr_ids.idx_rotation_quaternion,
            attr_ids.idx_billboard_ids,
            attr_ids.idx_texture_layer,
            task_location);
    }
    assert_true(cva->triangle_bone_weights.empty() == !triangles_res_->skeleton);
    if (triangles_res_->skeleton != nullptr) {
        if (va.bone_weight_buffer().is_awaited()) {
            va.bone_weight_buffer().bind();
        } else {
            UUVector<FixedArray<ShaderBoneWeight, 3>> triangle_bone_weights(cva->triangle_bone_weights.size());
            for (size_t tid = 0; tid < triangle_bone_weights.size(); ++tid) {
                const auto& td = cva->triangle_bone_weights[tid];  // std::vector of bone weights.
                auto& ts = triangle_bone_weights[tid];             // FixedArray of sorted bone weights.
                for (size_t vid = 0; vid < CW::length(td); ++vid) {
                    auto vd = td(vid);   // Copy of std::vector of bone weights, to be sorted.
                    auto& vs = ts(vid);  // Reference to FixedArray of sorted bone weights.
                    // Sort in descending order
                    std::sort(vd.begin(), vd.end(), [](const BoneWeight& w0, const BoneWeight& w1){return w0.weight > w1.weight;});
                    float sum_weights = 0;
                    for (size_t i = 0; i < ANIMATION_NINTERPOLATED; ++i) {
                        if (i < vd.size()) {
                            if (vd[i].bone_index >= triangles_res_->bone_indices.size()) {
                                THROW_OR_ABORT(
                                    "Bone index too large in get_vertex_array: " +
                                    std::to_string(vd[i].bone_index) + " >= " +
                                    std::to_string(triangles_res_->bone_indices.size()));
                            }
                            if (vd[i].bone_index > 255) {
                                THROW_OR_ABORT("Bone index too large for unsigned char");
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
                        THROW_OR_ABORT("Sum of weights too small");
                    }
                    if (sum_weights > 1.1) {
                        THROW_OR_ABORT("Sum of weights too large");
                    }
                    for (float& weight : vs.weights) {
                        weight /= sum_weights;
                    }
                }
            }
            // The "triangle_bone_weights" array is temporary, so wait until it is transferred.
            va.bone_weight_buffer().set(triangle_bone_weights, TaskLocation::FOREGROUND);
        }

        ShaderBoneWeight* bw = nullptr;
        CHK(glEnableVertexAttribArray(attr_ids.idx_bone_indices));
        CHK(glVertexAttribIPointer(attr_ids.idx_bone_indices, ANIMATION_NINTERPOLATED, GL_UNSIGNED_BYTE, sizeof(ShaderBoneWeight), &bw->indices));
        CHK(glEnableVertexAttribArray(attr_ids.idx_bone_weights));
        CHK(glVertexAttribPointer(attr_ids.idx_bone_weights, ANIMATION_NINTERPOLATED, GL_FLOAT, GL_FALSE, sizeof(ShaderBoneWeight), &bw->weights));
    }
    if (va.has_continuous_triangle_texture_layers() &&
        va.has_discrete_triangle_texture_layers())
    {
        THROW_OR_ABORT("Detected both, discrete and continuous texture layers");
    }
    if (va.has_continuous_triangle_texture_layers()) {
        if (va.texture_layer_buffer().is_awaited()) {
            va.texture_layer_buffer().bind();
        } else {
            if (cva->continuous_triangle_texture_layers.size() != cva->triangles.size()) {
                THROW_OR_ABORT("#continuous_triangle_texture_layers != #triangles");
            }
            va.texture_layer_buffer().set(cva->continuous_triangle_texture_layers, task_location);
        }

        CHK(glEnableVertexAttribArray(attr_ids.idx_texture_layer));
        CHK(glVertexAttribPointer(attr_ids.idx_texture_layer, 1, GL_FLOAT, GL_FALSE, sizeof(float), nullptr));
    }
    if (va.has_discrete_triangle_texture_layers()) {
        if (va.texture_layer_buffer().is_awaited()) {
            va.texture_layer_buffer().bind();
        } else {
            if (cva->discrete_triangle_texture_layers.size() != cva->triangles.size()) {
                THROW_OR_ABORT("#discrete_triangle_texture_layers != #triangles");
            }
            va.texture_layer_buffer().set(cva->discrete_triangle_texture_layers, task_location);
        }

        CHK(glEnableVertexAttribArray(attr_ids.idx_texture_layer));
        CHK(glVertexAttribIPointer(attr_ids.idx_texture_layer, 1, GL_UNSIGNED_BYTE, sizeof(uint8_t), nullptr));
    }
    if (!cva->material.interior_textures.empty()) {
        if (va.interior_mapping_buffer().is_awaited()) {
            va.interior_mapping_buffer().bind();
        } else {
            if (cva->triangles.size() != cva->interiormap_uvmaps.size()) {
                THROW_OR_ABORT(
                    (std::stringstream() << "#triangles: " << cva->triangles.size() <<
                    ", #interiormap_uvmap: " << cva->interiormap_uvmaps.size()).str());
            }
            std::vector<ShaderInteriorMappedFacade> shader_interior_mapped_facade;
            shader_interior_mapped_facade.reserve(3 * cva->triangles.size());
            for (const auto& [i, t] : enumerate(cva->triangles)) {
                for (size_t j = 0; j < 3; ++j) {
                    shader_interior_mapped_facade.push_back(ShaderInteriorMappedFacade{
                        .bottom_left = t(0).position,
                        .uvmap = cva->interiormap_uvmaps[i]
                        });
                }
            }
            // The "shader_interior_mapped_facade" array is temporary, so wait until it is transferred.
            va.interior_mapping_buffer().set(shader_interior_mapped_facade, TaskLocation::FOREGROUND);
        }

        ShaderInteriorMappedFacade* im = nullptr;
        CHK(glEnableVertexAttribArray(attr_ids.idx_interior_mapping_bottom_left));
        CHK(glVertexAttribPointer(attr_ids.idx_interior_mapping_bottom_left, 3, GL_FLOAT, GL_FALSE, sizeof(ShaderInteriorMappedFacade), &im->bottom_left));
        CHK(glEnableVertexAttribArray(attr_ids.idx_interior_mapping_uvmap));
        CHK(glVertexAttribPointer(attr_ids.idx_interior_mapping_uvmap, 4, GL_FLOAT, GL_FALSE, sizeof(ShaderInteriorMappedFacade), &im->uvmap));
    }

    CHK(glBindVertexArray(0));
    if (pva == nullptr) {
        return *vertex_arrays_.add(cva.get(), std::move(si));
    } else {
        return **pva;
    }
}

std::list<std::shared_ptr<AnimatedColoredVertexArrays>> ColoredVertexArrayResource::get_rendering_arrays() const
{
    return { triangles_res_ };
}

std::list<TypedMesh<std::shared_ptr<IIntersectable>>> ColoredVertexArrayResource::get_intersectables() const
{
    return {};
}

void ColoredVertexArrayResource::set_absolute_joint_poses(
    const UUVector<OffsetAndQuaternion<float, float>>& poses)
{
    for (auto& t : triangles_res_->scvas) {
        t = t->transformed<float>(poses, "_transformed_oq");
    }
    if (!triangles_res_->dcvas.empty()) {
        THROW_OR_ABORT("Poses only supported for single precision");
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

bool ColoredVertexArrayResource::copy_in_progress() const {
    // for (const auto& [i, cva] : enumerate(triangles_res_->scvas)) {
    for (const auto& cva : triangles_res_->scvas) {
        if (get_vertex_array(cva, TaskLocation::BACKGROUND).copy_in_progress()) {
            // linfo() << this << " - vertex copy " << i << " " << cva->name;
            return true;
        }
        if ((instances_ != nullptr) && (instances_->at(cva.get())->copy_in_progress())) {
            // linfo() << this << " - instances copy " << i << " " << cva->name;
            return true;
        }
    }
    return false;
}

void ColoredVertexArrayResource::wait() const {
    for (const auto& cva : triangles_res_->scvas) {
        get_vertex_array(cva, TaskLocation::FOREGROUND);
        if (instances_ != nullptr) {
            instances_->at(cva.get())->wait();
        }
    }
}
