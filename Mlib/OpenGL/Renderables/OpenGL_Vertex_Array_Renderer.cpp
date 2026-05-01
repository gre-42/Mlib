#include "OpenGL_Vertex_Array_Renderer.hpp"
#include <Mlib/Geometry/Material/Interior_Texture_Set.hpp>
#include <Mlib/Geometry/Material/Texture_Target.hpp>
#include <Mlib/Geometry/Material_Features.hpp>
#include <Mlib/Geometry/Mesh/Mesh_Meta.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Iterator/Enumerate.hpp>
#include <Mlib/Json/Json_Object_File.hpp>
#include <Mlib/Map/Ordered_Map.hpp>
#include <Mlib/Math/Transformation/Quaternion.hpp>
#include <Mlib/Misc/Floating_Point_Exceptions.hpp>
#include <Mlib/Misc/Log.hpp>
#include <Mlib/OpenGL/Batch_Renderers/Infer_Shader_Properties.hpp>
#include <Mlib/OpenGL/CHK.hpp>
#include <Mlib/OpenGL/Frame_Index_From_Animation_Time.hpp>
#include <Mlib/OpenGL/Gen_Shader_Text.hpp>
#include <Mlib/OpenGL/Instance_Handles/Colored_Render_Program.hpp>
#include <Mlib/OpenGL/Instance_Handles/Render_Guards.hpp>
#include <Mlib/OpenGL/Instance_Handles/Texture_Binder.hpp>
#include <Mlib/OpenGL/Render_Config.hpp>
#include <Mlib/OpenGL/Resource_Managers/Rendering_Resources.hpp>
#include <Mlib/OpenGL/Resources/Colored_Vertex_Array_Resource.hpp>
#include <Mlib/OpenGL/Resources/Colored_Vertex_Array_Resource/Shader_Structs.hpp>
#include <Mlib/OpenGL/Shader_Version_3_0.hpp>
#include <Mlib/OpenGL/Toggle_Benchmark_Rendering.hpp>
#include <Mlib/Os/Env.hpp>
#include <Mlib/Scene_Graph/Culling/Frustum_Visibility_Check.hpp>
#include <Mlib/Scene_Graph/Culling/Instances_Are_Visible.hpp>
#include <Mlib/Scene_Graph/Culling/Visibility_Check.hpp>
#include <Mlib/Scene_Graph/Elements/Animation_State.hpp>
#include <Mlib/Scene_Graph/Elements/Color_Style.hpp>
#include <Mlib/Scene_Graph/Elements/Dynamic_Style.hpp>
#include <Mlib/Scene_Graph/Elements/Light.hpp>
#include <Mlib/Scene_Graph/Elements/Skidmark.hpp>
#include <Mlib/Scene_Graph/Render/Attribute_Index_Calculator.hpp>
#include <Mlib/Scene_Graph/Render/IGpu_Instance_Buffers.hpp>
#include <Mlib/Scene_Graph/Render/IGpu_Vertex_Array.hpp>
#include <Mlib/Scene_Graph/Render/IGpu_Vertex_Data.hpp>
#include <Mlib/Scene_Graph/Render_Pass.hpp>
#include <Mlib/Strings/String.hpp>
#include <Mlib/Testing/Assert.hpp>

using namespace Mlib;

static const auto dirtmap_name = VariableAndHash<std::string>{ "dirtmap" };

struct IdAndTexture {
    size_t id;
    const ITextureHandle& texture;
};

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
    NotSortedUvMap(OrderedStandardMap<UvMapKey, size_t>& m): m_{ m }
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
    OrderedStandardMap<UvMapKey, size_t>& m_;
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

enum class ReductionTarget {
    COLOR,
    ALPHA
};

static GenShaderText vertex_shader_text_gen{[](
    const NotSortedArray<std::vector<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<Light>>>>& lights,
    const NotSortedArray<std::vector<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<Skidmark>>>>& skidmarks,
    const NotSortedArray<std::vector<BlendMapTextureAndId>>& textures_color,
    const NotSortedArray<std::vector<BlendMapTextureAndId>>& textures_alpha,
    const NotSortedArray<std::vector<size_t>>& lightmap_indices,
    const NotSortedStruct<ExternalRenderPassType>& render_pass,
    const NotSortedStruct<AttributeIndexCalculator>& attr_idc,
    const AttributeIndices& attr_ids,
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
    bool has_flat,
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
                    throw std::runtime_error("UV index too large");
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
                throw std::runtime_error("Unknown blend-map UV source");
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
    if (has_flat) {
        sstr << "uniform mat3 lookat;" << std::endl;
    }
    sstr << "layout (location=" << attr_ids.idx_position << ") in vec3 vPos;" << std::endl;
    sstr << "layout (location=" << attr_ids.idx_color << ") in vec3 vCol;" << std::endl;
    assert_true(attr_idc->nuvs == attr_ids.uv_count);
    assert_true(
        (attr_idc->nuvs == nuv_indices) ||
        ((nuv_indices == 1) && (attr_idc->nuvs != 0)));
    for (size_t i = 0; i < nuv_indices; ++i) {
        sstr << "layout (location=" << (attr_ids.idx_uv_0 + i) << ") in vec2 vTexCoord" << i << ";" << std::endl;
    }
    if (ncweight_indices > attr_ids.cweight_count) {
        throw std::runtime_error("CWeight index too large");
    }
    assert_true(attr_idc->ncweights == ncweight_indices);
    for (size_t i = 0; i < ncweight_indices; ++i) {
        sstr << "layout (location=" << (attr_ids.idx_cweight_0 + i) << ") in float vCWeight" << i << ";" << std::endl;
    }
    assert_true(attr_idc->has_alpha == has_alpha);
    if (has_alpha) {
        sstr << "layout (location=" << (attr_ids.idx_alpha) << ") in float vAlpha;" << std::endl;
    }
    if (reorient_uv0 || has_diffusivity || has_nontrivial_specularity || has_fresnel_exponent || has_interiormap || fragments_depend_on_normal || (!reflectance.all_equal(0.f) && !reflect_only_y)) {
        assert_true(attr_idc->has_normal);
        sstr << "layout (location=" << attr_ids.idx_normal << ") in vec3 vNormal;" << std::endl;
    }
    if (has_normalmap || has_interiormap) {
        assert_true(attr_idc->has_tangent);
        sstr << "layout (location=" << attr_ids.idx_tangent << ") in vec3 vTangent;" << std::endl;
    }
    assert_true(attr_idc->has_instance_attrs == has_instances);
    if (attr_idc->has_instance_attrs) {
        if (has_yangle) {
            sstr << "layout (location=" << attr_ids.idx_instance_attrs << ") in vec4 instancePosition;" << std::endl;
        } else {
            sstr << "layout (location=" << attr_ids.idx_instance_attrs << ") in vec3 instancePosition;" << std::endl;
        }
        if (has_rotation_quaternion) {
            assert_true(attr_idc->has_rotation_quaternion);
            sstr << "layout (location=" << attr_ids.idx_rotation_quaternion << ") in vec4 rotationQuaternion;" << std::endl;
        }
    } else if (has_lookat && !orthographic) {
        sstr << "const vec3 instancePosition = vec3(0.0, 0.0, 0.0);" << std::endl;
    }
    assert_true(attr_idc->has_billboard_ids == (nbillboard_ids != 0));
    if (attr_idc->has_billboard_ids) {
        sstr << "layout (location=" << attr_ids.idx_billboard_ids << ") in mediump uint billboard_id;" << std::endl;
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
        sstr << "layout (location=" << attr_ids.idx_bone_indices << ") in lowp uvec" << ANIMATION_NINTERPOLATED << " bone_ids;" << std::endl;
        sstr << "layout (location=" << attr_ids.idx_bone_weights << ") in vec" << ANIMATION_NINTERPOLATED << " bone_weights;" << std::endl;
        sstr << "uniform vec3 bone_positions[" << nbones << "];" << std::endl;
        sstr << "uniform vec4 bone_quaternions[" << nbones << "];" << std::endl;
    }
    assert_true(attr_idc->has_texture_layer == any(texture_layer_properties & TextureLayerProperties::VERTEX));
    if (any(texture_layer_properties & TextureLayerProperties::VERTEX)) {
        if (any(texture_layer_properties & TextureLayerProperties::DISCRETE)) {
            sstr << "layout (location=" << attr_ids.idx_texture_layer << ") in lowp uint texture_layer;" << std::endl;
        }
        if (any(texture_layer_properties & TextureLayerProperties::CONTINUOUS)) {
            sstr << "layout (location=" << attr_ids.idx_texture_layer << ") in float texture_layer;" << std::endl;
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
            throw std::runtime_error("No lights despite has_lightmap_color or has_lightmap_depth");
        }
        for (size_t i : lightmap_indices) {
            sstr << "uniform mat4 MVP_light" << i << ";" << std::endl;
            if (i > lights.size()) {
                throw std::runtime_error("Light index out of bounds");
            }
            const auto& l = *lights[i].second;
            if (!l.vp.has_value()) {
                throw std::runtime_error("Light has no projection matrix");
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
        sstr << "layout (location=" << attr_ids.idx_interior_mapping_bottom_left << ") in vec3 interior_bottom_left;" << std::endl;
        sstr << "layout (location=" << attr_ids.idx_interior_mapping_uvmap << ") in vec4 interior_uvmap;" << std::endl;
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
    //     throw std::runtime_error("has_lookat requires has_instances");
    // }
    if (has_yangle && !has_instances) {
        throw std::runtime_error("has_yangle requires has_instances");
    }
    if (has_flat || has_lookat || has_yangle || has_rotation_quaternion) {
        if (has_rotation_quaternion) {
            sstr << "    vPosInstance = rotate(rotationQuaternion, vPosInstance);" << std::endl;
        } else {
            if (!has_flat) {
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
            }
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
    } else if (has_instances) {
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
            throw std::runtime_error("Light index too large");
        }
        const auto& light = *lights[i].second;
        if (!light.vp.has_value()) {
            throw std::runtime_error("Light has no projection matrix");
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
            throw std::runtime_error("Skidmark projection matrix not orthographic");
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
        linfo() << external_render_pass_type_to_string(*render_pass);
        if (!textures_color.empty()) {
            linfo() << "Color: " + textures_color[0]->texture_descriptor.color.filename.string();
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
        throw std::runtime_error("Incompatible texture layer sizes");
    }
    if (left >= right) {
        throw std::runtime_error("Invalid interpolation bounds");
    }
    if (left >= continuous_layer_x.size()) {
        throw std::runtime_error("Invalid left boundary");
    }
    if (right >= continuous_layer_x.size()) {
        throw std::runtime_error("Invalid right boundary");
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
    bool blend,
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
        throw std::runtime_error("UV index too large");
    }
    if (ncweights > attr_ids->cweight_count) {
        throw std::runtime_error("CWeight index too large");
    }
    // Mipmapping does not work unless all textures are actually sampled everywhere.
    bool compute_interiormap_at_end = true;
    if (std::isnan(alpha_threshold)) {
        throw std::runtime_error("alpha_threshold is NAN => unknown blend mode");
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
            throw std::runtime_error("Specular maps not supported for blended textures");
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
    for (size_t i = 0; i < uv_map.size(); ++i) {
        sstr << "in vec2 tex_coord" << i << ";" << std::endl;
    }
    for (size_t i = 0; i < ncweights; ++i) {
        sstr << "in float cweight" << i << ";" << std::endl;
    }
    sstr << "out vec4 frag_color;" << std::endl;
    if (has_alpha || ((nbillboard_ids != 0) && !orthographic)) {
        sstr << "centroid in float alpha_fac_v;" << std::endl;
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
                    throw std::runtime_error("Unsupported mipmap modes in texture \"" + t->texture_descriptor.color.filename.string() + '"');
                }
            }
        } else if (mip2) {
            throw std::runtime_error(
                "Color: 2D mipmaps require a texture layer: \"" +
                textures_color[0]->texture_descriptor.color.filename.string() + '"');
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
                    throw std::runtime_error("Unsupported mipmap modes in texture \"" + t->texture_descriptor.color.filename.string() + '"');
                }
            }
        } else if (mip2) {
            throw std::runtime_error(
                "Alpha: 2D mipmaps require a texture layer: \"" +
                textures_alpha[0]->texture_descriptor.color.filename.string() + '"');
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
            throw std::runtime_error("No lights despite has_lightmap_color or has_lightmap_depth");
        }
        for (size_t i : lightmap_indices) {
            if (i > lights.size()) {
                throw std::runtime_error("Light index too large");
            }
            const auto& light = *lights[i].second;
            if (!light.vp.has_value()) {
                throw std::runtime_error("Light has no projection matrix");
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
                    throw std::runtime_error("Unsupported mipmap modes in texture \"" + t->texture_descriptor.normal.filename.string() + '"');
                }
            }
        } else if (mip2) {
            throw std::runtime_error(
                "Normalmap: 2D mipmaps require a texture layer: \"" +
                textures_color[0]->texture_descriptor.normal.filename.string() + '"');
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
                if (t->texture_descriptor.color.filename.empty()) {
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
                if (t->texture_descriptor.normal.filename.empty()) {
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
                if (t->texture_descriptor.color.filename.empty()) {
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
#if defined(__ANDROID__) || defined(__EMSCRIPTEN__)
        sstr << "    int idx = 2 * best_axis + int(best_sign);";
        for (size_t i = 0; i < size(InteriorTextureSet::INTERIOR_COLORS); ++i) {
            sstr << "    if (idx == " << i << ") frag_color = texture(texture_interior[" << i << "], best_uv);" << std::endl;
        }
#else
        sstr << "    frag_color = texture(texture_interior[2 * best_axis + int(best_sign)], best_uv);" << std::endl;
#endif
        if (any(interior_texture_set & InteriorTextureSet::BACK_SPECULAR)) {
            if (!has_reflection_map) {
                throw std::runtime_error("Back specular texture requires reflection map");
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
                throw std::runtime_error("Front specular texture requires reflection map");
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
            sstr << "    if (!gl_FrontFacing) {" << std::endl;
            // if (orthographic) {
            //     sstr << "    if (dot(norm, viewDir) < 0.0) {" << std::endl;
            // } else {
            //     // From: https://stackoverflow.com/questions/2523439/ipad-glsl-from-within-a-fragment-shader-how-do-i-get-the-surface-not-vertex
            //     sstr << "    vec3 normalvector = cross(dFdx(FragPos), dFdy(FragPos));" << std::endl;
            //     sstr << "    if (dot(norm, normalvector) < 0.0) {" << std::endl;
            // }
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
                // throw std::runtime_error("Distances not supported by orthographic projection");
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
        throw std::runtime_error("Incompatible texture layer sizes");
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
            throw std::runtime_error("Unsupported base blend map role (0)");
        }
    }
    if (alpha_threshold != 0) {
        if (textures_color.empty()) {
            throw std::runtime_error("Alpha threshold requires texture");
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
                if (textures_color[0]->texture_descriptor.normal.filename.empty()) {
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
                    throw std::runtime_error("cweight index too large");
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
                        default: throw std::runtime_error("Unknown detail mask");
                    }
                }();
                sstr << "            float w = " << sample_color(i) << '.' << c << ';' << std::endl;
                if (t->reduction == BlendMapReductionOperation::TIMES) {
                    sstr << "            weight *= w;" << std::endl;
                } else if (t->reduction == BlendMapReductionOperation::FEATHER) {
                    if (t->discreteness == 0) {
                        throw std::runtime_error("Detail-mask with feather has zero discreteness");
                    }
                    sstr << "            weight += (0.5 - abs(weight - 0.5)) * (w + " << t->plus << ") * " << t->discreteness << ';' << std::endl;
                    sstr << "            weight = clamp(weight, 0.0, 1.0);" << std::endl;
                } else if (t->reduction == BlendMapReductionOperation::INVERT) {
                    sstr << "            weight = 1.0 - weight;" << std::endl;
                } else {
                    throw std::runtime_error("Unknown reduction operation");
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
                    throw std::runtime_error("Alpha-texture not loaded as grayscale");
                }
                sstr << "            float intensity = " << sample_alpha(i) << ".r;" << std::endl;
                sstr << "            float final_weight = weight;" << std::endl;
            } else {
                throw std::runtime_error("Texture: \"" + t->texture_descriptor.color.filename.string() + "\". Unsupported color mode: \"" + color_mode_to_string(t->texture_descriptor.color.color_mode) + '"');
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
                    default: throw std::runtime_error("Unknown blendmap reduction type");
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
                        throw std::runtime_error("Unknown reduction target (1)");
                    }
                } else if (!any(t->role & BlendMapRole::ANY_DETAIL_MASK)) {
                    throw std::runtime_error("Unknown blend map role");
                }
                if (target == ReductionTarget::COLOR) {
                    if (has_normalmap) {
                        if (t->texture_descriptor.normal.filename.empty()) {
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
            throw std::runtime_error("Target is alpha and reweighting is not undefined in texture \"" + textures[0]->texture_descriptor.color.filename.string() + '"');
        }
        if ((target == ReductionTarget::COLOR) && (textures[0]->reweight_mode == BlendMapReweightMode::UNDEFINED)) {
            throw std::runtime_error("Target is color and reweighting is undefined in texture \"" + textures[0]->texture_descriptor.color.filename.string() + '"');
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
            throw std::runtime_error("Unsupported base blend map role (1)");
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
            throw std::runtime_error("Light index too large");
        }
        const auto& light = *lights[i].second;
        if (!light.vp.has_value()) {
            throw std::runtime_error("Light has no projection matrix");
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
                    throw std::runtime_error("Particle type \"none\" does not require a skidmark");
                case ParticleType::SMOKE:
                    throw std::runtime_error("Smoke does not require a skidmark");
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
                throw std::runtime_error("Unknown particle type");
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
        throw std::runtime_error("Combination of ((ntextures_color == 0) && has_dirtmap) is not supported");
    }
    if (has_dirtmap) {
        sstr << "    float dirtiness = texture(texture_dirtmap, tex_coord_dirtmap).r;" << std::endl;
        sstr << "    vec4 dirt_color = texture(texture_dirt, tex_coord_dirt);" << std::endl;
        if (dirt_color_mode == ColorMode::RGBA) {
            sstr << "    dirtiness *= dirt_color.a;" << std::endl;
        } else if (dirt_color_mode != ColorMode::RGB) {
            throw std::runtime_error("Unsupported dirt color mode: " + color_mode_to_string(dirt_color_mode));
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
    if (!blend) {
        sstr << "    frag_color.a = 1.0;" << std::endl;
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
        linfo() << external_render_pass_type_to_string(render_pass);
        if (!textures_color.empty()) {
            linfo() << "Color: " + textures_color[0]->texture_descriptor.color.filename.string();
        }
        std::string line;
        for (size_t i = 1; std::getline(sstr, line); ++i) {
            linfo() << i << ": " << line;
        }
    }
    return sstr.str();
}};

OpenGLVertexArrayRenderer::OpenGLVertexArrayRenderer(
    RenderingResources& primary_rendering_resources,
    RenderingResources& secondary_rendering_resources)
    : primary_rendering_resources_{ primary_rendering_resources }
    , secondary_rendering_resources_{ secondary_rendering_resources }
{}

void OpenGLVertexArrayRenderer::render(
    const std::shared_ptr<IGpuVertexArray>& cva,
    const std::shared_ptr<AnimatedColoredVertexArrays>& acvas,
    const UUVector<OffsetAndQuaternion<float, float>>& absolute_bone_transformations,
    const FixedArray<ScenePos, 4, 4>& mvp,
    const TransformationMatrix<SceneDir, ScenePos, 3>& m,
    const TransformationMatrix<SceneDir, ScenePos, 3>& iv,
    const DynamicStyle* dynamic_style,
    const std::list<std::pair<TransformationMatrix<SceneDir, ScenePos, 3>, std::shared_ptr<Light>>>& lights,
    const std::list<std::pair<TransformationMatrix<SceneDir, ScenePos, 3>, std::shared_ptr<Skidmark>>>& skidmarks,
    const SceneGraphConfig& scene_graph_config,
    const RenderConfig& render_config,
    const RenderPass& render_pass,
    const AnimationState* animation_state,
    const ColorStyle* color_style) const
{
    // AperiodicLagFinder lag_finder{ "render_cva " + meta.name + ": ", std::chrono::milliseconds{5} };
    LOG_FUNCTION("render_cva");
    LOG_INFO("RenderableColoredVertexArray::render_cva " + cva->identifier());
    TIME_GUARD_DECLARE(time_guard, "render_cva", cva->identifier());
    // lerr() << external_render_pass_type_to_string(render_pass.rsd.external_render_pass.pass) << " " << cva->identifier();
    // if (rcva_->instances_ != nullptr) {
    //     lerr() << ", #inst: " << rcva_->instances_->at(cva.get())->num_instances();
    // }
    auto vertices = cva->vertices();
    auto instances = cva->instances();
    const auto& meta = vertices->mesh_meta();
    // This check passes because the arrays are filtered in the constructor.
    assert_true((meta.material.aggregate_mode == AggregateMode::NONE) || (instances != nullptr));
    if (any(meta.material.blend_mode & BlendMode::ANY_CONTINUOUS)) {
        if (render_pass.internal == InternalRenderPass::INITIAL) {
            // lerr() << ", skipped (0)";
            return;
        }
        if ((render_pass.internal == InternalRenderPass::BLENDED_EARLY) && !any(meta.material.blending_pass & BlendingPassType::EARLY)) {
            return;
        }
        if ((render_pass.internal == InternalRenderPass::BLENDED_LATE) && !any(meta.material.blending_pass & BlendingPassType::LATE)) {
            return;
        }
    } else if (any(render_pass.internal & InternalRenderPass::ANY_BLENDED)) {
        // lerr() << ", skipped (1)";
        return;
    }
    // This is now done in the VisibilityCheck class.
    // if (render_pass.rsd.external_render_pass.pass == ExternalRenderPassType::LIGHTMAP_TO_TEXTURE && render_pass.rsd.external_render_pass.black_node_name.empty() && meta.material.occluder_pass == OccluderType::OFF) {
    //     return;
    // }
    auto mvp_f = mvp.casted<float>();
    VisibilityCheck vc{ mvp_f };
    if (instances == nullptr) {
        FrustumVisibilityCheck fvc{vc};
        if (!fvc.is_visible(meta.name.full_name_and_hash(), meta.material, meta.morphology, BILLBOARD_ID_NONE, scene_graph_config, render_pass.rsd.external_render_pass.pass, cva->aabb()))
        {
            // lerr() << ", skipped (2)";
            return;
        }
    } else {
        if (!instances_are_visible(meta.material, render_pass.rsd.external_render_pass.pass)) {
            // lerr() << ", skipped (3)";
            return;
        }
        if (instances->num_instances() == 0) {
            // lerr() << ", skipped (4)";
            return;
        }
    }
    float texture_layer = NAN;
    if (meta.material.has_animated_textures) {
        if ((animation_state == nullptr) ||
            (animation_state->reference_time.index() == 0))
        {
            throw std::runtime_error(
                "Material has animated textures, but the animation state "
                "is not set or the (a)periodic reference time is inactive");
        }
        assert_true(animation_state != nullptr);
        texture_layer = std::visit(
            [&]<class RefTime>(const RefTime& reference_time) -> float {
                if constexpr (!std::is_same_v<RefTime, std::monostate>) {
                    return reference_time.phase01(render_pass.rsd.external_render_pass.time);
                }
                throw std::runtime_error("Reference time not set for material with textures: " + meta.material.identifier());
            },
            animation_state->reference_time);
        if (texture_layer < 0.f) {
            return;
        }
        if (texture_layer > 1.f) {
            throw std::runtime_error("Attempt to render outdated texture layertexture_layer");
        }
        if (std::isnan(texture_layer)) {
            verbose_abort("texture_layer is unexpectedly NAN");
        }
    }
    // lerr() << ", not skipped";

    std::vector<std::pair<TransformationMatrix<SceneDir, ScenePos, 3>, std::shared_ptr<Light>>> filtered_lights;
    std::vector<size_t> lightmap_indices;
    std::vector<size_t> light_noshadow_indices;
    std::vector<size_t> light_shadow_indices;
    std::vector<size_t> black_shadow_indices;
    std::vector<std::pair<TransformationMatrix<SceneDir, ScenePos, 3>, std::shared_ptr<Skidmark>>> filtered_skidmarks;
    bool is_lightmap = any(render_pass.rsd.external_render_pass.pass & ExternalRenderPassType::LIGHTMAP_ANY_MASK);
    FixedArray<float, 3> fog_emissive = fixed_zeros<float, 3>();
    FixedArray<float, 3> emissive = fixed_zeros<float, 3>();
    FixedArray<float, 3> ambient = fixed_zeros<float, 3>();
    FixedArray<float, 3> diffuse = fixed_zeros<float, 3>();
    FixedArray<float, 3> specular = fixed_zeros<float, 3>();
    float specular_exponent = 0.f;
    FixedArray<float, 3> fresnel_ambient = fixed_zeros<float, 3>();
    if (!is_lightmap) {
        emissive = color_style && !color_style->emissive.all_equal(-1.f)
            ? color_style->emissive
            : meta.material.shading.emissive;
        ambient = color_style && !color_style->ambient.all_equal(-1.f) ? color_style->ambient * meta.material.shading.ambient : meta.material.shading.ambient.array();
        diffuse = color_style && !color_style->diffuse.all_equal(-1.f) ? color_style->diffuse * meta.material.shading.diffuse : meta.material.shading.diffuse.array();
        specular = color_style && !color_style->specular.all_equal(-1.f) ? color_style->specular * meta.material.shading.specular : meta.material.shading.specular.array();
        specular_exponent = color_style && (color_style->specular_exponent != -1.f) ? color_style->specular_exponent : meta.material.shading.specular_exponent;
        fresnel_ambient = color_style && !color_style->fresnel_ambient.all_equal(-1.f)
            ? color_style->fresnel_ambient * meta.material.shading.fresnel.ambient
            : meta.material.shading.fresnel.ambient.array();
    } else {
        emissive = 1.f;
    }
    if (!is_lightmap) {
        filtered_lights.reserve(lights.size());
        light_noshadow_indices.reserve(lights.size());
        light_shadow_indices.reserve(lights.size());
        black_shadow_indices.reserve(lights.size());
        lightmap_indices.reserve(lights.size());
        for (const auto& tl : lights) {
            const auto& l = *tl.second;
            // By this definition, objects are occluded/lighted (occluded_pass)
            // by several shadowmaps/lightmaps (low-resolution and high-resolution shadowmaps).
            // The occluder_pass is checked in the "VisibilityCheck" class.
            if (meta.material.occluded_pass < l.shadow_render_pass) {
                continue;
            }
            bool light_emits_colors = l.emits_colors();
            bool light_casts_shadows = any(l.shadow_render_pass & ExternalRenderPassType::LIGHTMAP_ANY_MASK);

            if (!light_casts_shadows) {
                if (!light_emits_colors) {
                    continue;
                }
                if (all(l.ambient * ambient == 0.f) &&
                    all(l.diffuse * diffuse == 0.f) &&
                    all(l.specular * specular == 0.f) &&
                    all(l.fresnel_ambient * fresnel_ambient == 0.f))
                {
                    continue;
                }
            }
            size_t i = filtered_lights.size();
            filtered_lights.push_back(tl);
            if (light_emits_colors) {
                if (light_casts_shadows) {
                    lightmap_indices.push_back(i);
                    light_shadow_indices.push_back(i);
                    if (any(meta.material.occluded_pass & ExternalRenderPassType::LIGHTMAP_COLOR_MASK) &&
                        (l.lightmap_color == nullptr))
                    {
                        throw std::runtime_error("Light with color shadows has no color texture");
                    }
                    if (any(meta.material.occluded_pass & ExternalRenderPassType::LIGHTMAP_DEPTH_MASK) &&
                        (l.lightmap_depth == nullptr))
                    {
                        throw std::runtime_error("Light with depth shadows has no depth texture");
                    }
                } else {
                    light_noshadow_indices.push_back(i);
                    if (l.lightmap_color != nullptr) {
                        throw std::runtime_error("Light without shadow has a color texture");
                    }
                    if (l.lightmap_depth != nullptr) {
                        throw std::runtime_error("Light without shadow has a depth texture");
                    }
                }
            } else {
                lightmap_indices.push_back(i);
                black_shadow_indices.push_back(i);
                if (any(meta.material.occluded_pass & ExternalRenderPassType::LIGHTMAP_COLOR_MASK) &&
                    (l.lightmap_color == nullptr))
                {
                    throw std::runtime_error("Black shadow has no color texture");
                }
                if (any(meta.material.occluded_pass & ExternalRenderPassType::LIGHTMAP_DEPTH_MASK) &&
                    (l.lightmap_depth == nullptr))
                {
                    throw std::runtime_error("Black shadow has no depth texture");
                }
            }
        }
        bool no_light_active = light_noshadow_indices.empty() && light_shadow_indices.empty();
        if (no_light_active) {
            filtered_lights.clear();
            black_shadow_indices.clear();
            lightmap_indices.clear();
        }
        if (any(meta.material.skidmarks) && (!no_light_active || any(emissive != 0.f))) {
            filtered_skidmarks.reserve(skidmarks.size());
            for (const auto& s : skidmarks) {
                if (any(meta.material.skidmarks & s.second->particle_type)) {
                    filtered_skidmarks.push_back(s);
                }
            }
        }
    }
    std::vector<BlendMapTextureAndId> blended_textures_color(meta.material.textures_color.size());
    OrderedUnorderedMap<ColormapPtr, IdAndTexture> texture_ids_color;
    OrderedUnorderedMap<ColormapPtr, IdAndTexture> texture_ids_specular;
    OrderedUnorderedMap<ColormapPtr, IdAndTexture> texture_ids_normal;
    for (size_t i = 0; i < blended_textures_color.size(); ++i) {
        const auto& c = meta.material.textures_color[i];
        auto& b = blended_textures_color[i];
        if (!c.texture_descriptor.color.filename.empty()) {
            const auto& texture = secondary_rendering_resources_.contains_texture(c.texture_descriptor.color)
                ? *secondary_rendering_resources_.get_texture(c.texture_descriptor.color)
                : *primary_rendering_resources_.get_texture(c.texture_descriptor.color);
            auto it = texture_ids_color.try_emplace(c.texture_descriptor.color, texture_ids_color.size(), texture);
            b.id_color = it.first->second.id;
            b.tex_color = &texture;
        } else {
            b.id_color = SIZE_MAX;
            b.tex_color = nullptr;
        }
        if (!c.texture_descriptor.specular.filename.empty()) {
            const auto& texture = *primary_rendering_resources_.get_texture(c.texture_descriptor.specular);
            auto it = texture_ids_specular.try_emplace(c.texture_descriptor.specular, texture_ids_specular.size(), texture);
            b.id_specular = it.first->second.id;
            b.tex_specular = &texture;
        } else {
            b.id_specular = SIZE_MAX;
            b.tex_specular = nullptr;
        }
        if (!c.texture_descriptor.normal.filename.empty()) {
            const auto& texture = *primary_rendering_resources_.get_texture(c.texture_descriptor.normal);
            auto it = texture_ids_normal.try_emplace(c.texture_descriptor.normal, texture_ids_normal.size(), texture);
            b.id_normal = it.first->second.id;
            b.tex_normal = &texture;
        } else {
            b.id_normal = SIZE_MAX;
            b.tex_normal = nullptr;
        }
        b.ops = &c;
    }
    std::vector<BlendMapTextureAndId> blended_textures_alpha(meta.material.textures_alpha.size());
    std::unordered_map<ColormapPtr, IdAndTexture> texture_ids_alpha;
    for (size_t i = 0; i < blended_textures_alpha.size(); ++i) {
        const auto& c = meta.material.textures_alpha[i];
        auto& b = blended_textures_alpha[i];
        assert_true(!c.texture_descriptor.color.filename.empty());
        const auto& texture = secondary_rendering_resources_.contains_texture(c.texture_descriptor.color)
            ? *secondary_rendering_resources_.get_texture(c.texture_descriptor.color, TextureRole::COLOR_FROM_DB)
            : *primary_rendering_resources_.get_texture(c.texture_descriptor.color, TextureRole::COLOR_FROM_DB);
        auto it = texture_ids_alpha.try_emplace(c.texture_descriptor.color, texture_ids_alpha.size(), texture);
        b.id_color = it.first->second.id;
        b.id_specular = SIZE_MAX;
        b.id_normal = SIZE_MAX;
        b.tex_color = &texture;
        b.tex_specular = nullptr;
        b.tex_normal = nullptr;
        b.ops = &c;
    }
    auto check_sanity_common = [&meta](const std::vector<BlendMapTexture>& textures){
        for (const auto& t : textures) {
            if (t.texture_descriptor.color.filename.empty()) {
                throw std::runtime_error("Empty color or alpha texture not supported, cva: " + meta.name.full_name());
            }
            if (t.texture_descriptor.color.color_mode == ColorMode::UNDEFINED) {
                throw std::runtime_error("Material's color or alpha texture \"" + t.texture_descriptor.color.filename.string() + "\" has undefined color mode");
            }
        }
    };
    check_sanity_common(meta.material.textures_color);
    if (meta.material.textures_alpha.empty()) {
        for (const auto& [i, t] : enumerate(meta.material.textures_color)) {
            if (i == 0) {
                auto alpha_required =
                    (meta.material.blend_mode != BlendMode::OFF) &&
                    !any(meta.material.blend_mode & BlendMode::ADD_MASK) &&
                    !vertices->has_alpha();
                if (!alpha_required && !any(t.texture_descriptor.color.color_mode & ColorMode::RGB)) {
                    throw std::runtime_error("Opaque material's color texture \"" + t.texture_descriptor.color.filename.string() + "\" was not loaded as RGB");
                }
                if (alpha_required && !any(t.texture_descriptor.color.color_mode & ColorMode::RGBA)) {
                    throw std::runtime_error("Transparent material's color texture \"" + t.texture_descriptor.color.filename.string() + "\" was not loaded as RGBA");
                }
            }
        }
    } else {
        check_sanity_common(meta.material.textures_alpha);
        if (meta.material.blend_mode == BlendMode::OFF) {
            throw std::runtime_error("Blend-mode is off despite alpha texture, cva: " + meta.name.full_name() + ", material: " + meta.material.identifier());
        }
        for (const auto& t : meta.material.textures_color) {
            if (t.texture_descriptor.color.color_mode != ColorMode::RGB) {
                throw std::runtime_error("Color-texture \"" + t.texture_descriptor.color.filename.string() + "\" was not loaded as RGB, but an alpha-texture was set");
            }
        }
        for (const auto& t : meta.material.textures_alpha) {
            if (t.texture_descriptor.color.color_mode != ColorMode::GRAYSCALE) {
                throw std::runtime_error("Alpha-texture \"" + t.texture_descriptor.color.filename.string() + "\" was not loaded as grayscale");
            }
        }
    }
    FixedArray<float, 3> fresnel_emissive = fixed_zeros<float, 3>();
    FresnelReflectance fresnel;
    bool any_light_has_ambient = false;
    bool any_light_has_diffuse = false;
    bool any_light_has_specular = false;
    if (!filtered_lights.empty() && !is_lightmap) {
        FixedArray<float, 3> sum_light_fresnel_ambient = fixed_zeros<float, 3>();
        FixedArray<float, 3> sum_light_fog_ambient = fixed_zeros<float, 3>();
        for (const auto& [_, light] : filtered_lights) {
            if (light->emits_colors()) {
                sum_light_fresnel_ambient += light->fresnel_ambient;
                sum_light_fog_ambient += light->fog_ambient;
                any_light_has_ambient |= any((light->ambient * ambient) != 0.f);
                any_light_has_diffuse |= any((light->diffuse * diffuse) != 0.f);
                any_light_has_specular |= any((light->specular * specular) != 0.f);
            }
        }
        fresnel_emissive = sum_light_fresnel_ambient * fresnel_ambient;
        if (any_light_has_specular || any(fresnel_emissive != 0.f)) {
            fresnel = color_style && (color_style->fresnel.exponent != -1.f) ? color_style->fresnel : meta.material.shading.fresnel.reflectance;
        }
        fog_emissive = sum_light_fog_ambient * meta.material.shading.fog_ambient;
    }
    if (!any_light_has_ambient) {
        ambient = 0.f;
    }
    if (!any_light_has_diffuse) {
        diffuse = 0.f;
    }
    if (!any_light_has_specular) {
        specular = 0.f;
        specular_exponent = 0.f;
    }
    if ((fresnel.exponent != 0.f) && (std::abs(fresnel.max - fresnel.min) < 1e-12)) {
        throw std::runtime_error("Nonzero fresnel exponent requires nonzero fresnel range");
    }
    if ((fresnel.exponent == 0.f) && ((fresnel.max != 0.f) || (fresnel.min != 0.f))) {
        throw std::runtime_error("Zero fresnel exponent requires zero fresnel coefficients");
    }
    if (is_lightmap || (filtered_lights.empty() && all(emissive == 0.f))) {
        if ((meta.material.blend_mode != BlendMode::OFF) && (meta.material.depth_func != DepthFunc::EQUAL))
        {
            if (!texture_ids_color.empty()) {
                assert_true(!blended_textures_color.empty());
                auto& b = blended_textures_color[0];
                assert_true(b.id_color == 0);
                assert_true(b.tex_color != nullptr);
                blended_textures_color.resize(1);
                texture_ids_color.clear();
                texture_ids_color.try_emplace(b.ops->texture_descriptor.color, 0, *b.tex_color);
            }
            texture_ids_specular.clear();
            texture_ids_normal.clear();
        } else {
            texture_ids_color.clear();
            texture_ids_specular.clear();
            texture_ids_normal.clear();
            texture_ids_alpha.clear();
            blended_textures_color.clear();
            blended_textures_alpha.clear();
        }
    }
    bool fragments_depend_on_normal =
        !is_lightmap &&
        Mlib::fragments_depend_on_normal(blended_textures_color);
    const VariableAndHash<std::string>* reflection_map = nullptr;
    FixedArray<float, 3> reflectance{ 0.f };
    if (!is_lightmap &&
        !meta.material.reflection_map.empty() &&
        (!meta.material.shading.reflectance.all_equal(0.f) ||
            any(meta.material.interior_textures.set & InteriorTextureSet::ANY_SPECULAR)))
    {
        if (color_style == nullptr) {
            throw std::runtime_error("cva \"" + meta.name.full_name() + "\": Material with reflection map \"" + meta.material.reflection_map.string() + "\" has no style");
        }
        auto it = color_style->reflection_maps.find(meta.material.reflection_map.variable_and_hash());
        if (it == color_style->reflection_maps.end()) {
            throw std::runtime_error(
                "cva \"" + meta.name.full_name() + "\": Could not find reflection map \""
                + meta.material.reflection_map.string()
                + "\" in style with keys:"
                + join(", ", color_style->reflection_maps, [](const auto& s){return *s.first;}));
        }
        if (!it->second->empty()) {
            reflection_map = &it->second;
            reflectance = meta.material.shading.reflectance * color_style->reflection_strength;
            if (any(reflectance <= 0.f) && !any(meta.material.interior_textures.set & InteriorTextureSet::ANY_SPECULAR)) {
                throw std::runtime_error("Reflectance is not positive, and no specular interior textures were specified");
            }
        }
    }
    if (!(meta.material.reorient_uv0 ||
          any(diffuse != 0.f) ||
          (specular_exponent != 0.f) ||
          (fresnel.exponent != 0.f) ||
          fragments_depend_on_normal ||
          (any(reflectance != 0.f) && !meta.material.reflect_only_y)))
    {
        texture_ids_normal.clear();
    }
    bool has_horizontal_detailmap = false;
    has_horizontal_detailmap |= Mlib::has_horizontal_detailmap(blended_textures_color);
    has_horizontal_detailmap |= Mlib::has_horizontal_detailmap(blended_textures_alpha);
    std::vector<size_t> lightmap_indices_color = any(meta.material.occluded_pass & ExternalRenderPassType::LIGHTMAP_COLOR_MASK) ? lightmap_indices : std::vector<size_t>{};
    std::vector<size_t> lightmap_indices_depth = any(meta.material.occluded_pass & ExternalRenderPassType::LIGHTMAP_DEPTH_MASK) ? lightmap_indices : std::vector<size_t>{};
    assert_true(lightmap_indices_color.empty() || lightmap_indices_depth.empty());
    for (const auto& [i, t] : enumerate(meta.material.textures_color)) {
        if ((i != 0) && !t.texture_descriptor.specular.filename.empty()) {
            throw std::runtime_error("Only the first texture can have a specularmap");
        }
    }
    size_t ntextures_reflection = (size_t)(!is_lightmap && (reflection_map != nullptr) && !(*reflection_map)->empty());
    size_t ntextures_dirt = ((!meta.material.dirt_texture.empty()) && !is_lightmap && !filtered_lights.empty()) ? 2 : 0;
    InteriorTextureSet interior_texture_set;
    size_t ntextures_interior;
    if (is_lightmap) {
        ntextures_interior = 0;
        interior_texture_set = InteriorTextureSet::NONE;
    } else {
        ntextures_interior = meta.material.interior_textures.size();
        interior_texture_set = meta.material.interior_textures.set;
        assert_true(size(interior_texture_set) == ntextures_interior);
    }
    bool has_instances = (instances != nullptr);
    bool has_flat = (meta.material.transformation_mode == TransformationMode::POSITION_FLAT);
    bool has_lookat = (meta.material.transformation_mode == TransformationMode::POSITION_LOOKAT);
    bool has_yangle = (meta.material.transformation_mode == TransformationMode::POSITION_YANGLE);
    bool has_rotation_quaternion = has_instances && (meta.material.transformation_mode == TransformationMode::ALL);
    OrderableFixedArray<float, 4> alpha_distances_common = meta.material.alpha_distances;
    OrderableFixedArray<float, 2> fog_distances = meta.material.shading.fog_distances;
    if (is_lightmap) {
        alpha_distances_common = default_linear_distances;
        fog_distances = default_step_distances;
    }
    if (all(fog_emissive == 0.f) &&
        all(ambient == 0.f) &&
        all(diffuse == 0.f) &&
        all(specular == 0.f) &&
        all(emissive == 0.f) &&
        all(fresnel_emissive == 0.f))
    {
        fog_distances = default_step_distances;
    }
    bool fragments_depend_on_distance = Mlib::fragments_depend_on_distance(
        fog_distances, alpha_distances_common, blended_textures_color);
    if (texture_ids_color.empty() && (ntextures_dirt != 0)) {
        throw std::runtime_error(
            "Combination of ((ntextures_color == 0) && (ntextures_dirt != 0)) is not supported. Textures: " +
            join(" ", meta.material.textures_color, [](const auto& v) { return v.texture_descriptor.color.filename.string(); }));
    }
    bool reorient_normals = !meta.material.cull_faces && (any(diffuse != 0.f) || any(specular != 0.f));
    if (meta.material.cull_faces && meta.material.reorient_uv0) {
        throw std::runtime_error("reorient_uv0 requires disabled face culling");
    }
    bool reorient_uv0 = meta.material.reorient_uv0 && !texture_ids_color.empty();
    bool has_dynamic_emissive = meta.material.dynamically_lighted && !any(render_pass.rsd.external_render_pass.pass & ExternalRenderPassType::LIGHTMAP_COLOR_MASK);
    LOG_INFO("RenderableColoredVertexArray::render_cva get_render_program");
    assert_true(meta.material.number_of_frames > 0);
    Hasher texture_modifiers_hash;
    texture_modifiers_hash.combine(blended_textures_color.size());
    for (const auto& t : blended_textures_color) {
        texture_modifiers_hash.combine(t.ops->modifiers_hash());
        texture_modifiers_hash.combine(t.id_color);
        texture_modifiers_hash.combine(t.id_normal);
        texture_modifiers_hash.combine(t.id_specular);
        texture_modifiers_hash.combine(t.tex_color == nullptr ? UINT32_MAX : t.tex_color->layers());
        texture_modifiers_hash.combine(t.tex_normal == nullptr ? UINT32_MAX : t.tex_normal->layers());
        texture_modifiers_hash.combine(t.tex_specular == nullptr ? UINT32_MAX : t.tex_specular->layers());
    }
    texture_modifiers_hash.combine(blended_textures_alpha.size());
    for (const auto& t : blended_textures_alpha) {
        texture_modifiers_hash.combine(t.ops->modifiers_hash());
        texture_modifiers_hash.combine(t.id_color);
        texture_modifiers_hash.combine(t.id_normal);
        texture_modifiers_hash.combine(t.id_specular);
        texture_modifiers_hash.combine(t.tex_color == nullptr ? UINT32_MAX : t.tex_color->layers());
        texture_modifiers_hash.combine(t.tex_normal == nullptr ? UINT32_MAX : t.tex_normal->layers());
        texture_modifiers_hash.combine(t.tex_specular == nullptr ? UINT32_MAX : t.tex_specular->layers());
    }
    Hasher lights_hash;
    lights_hash.combine(filtered_lights.size());
    for (const auto& [_, l] : filtered_lights) {
        lights_hash.combine(l->shading_hash());
    }
    Hasher skidmarks_hash;
    skidmarks_hash.combine(filtered_skidmarks.size());
    for (const auto& [_, s] : filtered_skidmarks) {
        skidmarks_hash.combine(s->shading_hash());
    }
    bool has_discrete_atlas_texture_layer = get_has_discrete_atlas_texture_layer(meta);
    if (has_discrete_atlas_texture_layer) {
        if (meta.material.textures_color.size() != 1) {
            throw std::runtime_error("Unexpected number of color textures");
        }
        const auto& t = meta.material.textures_color[0];
        auto ttype = secondary_rendering_resources_.contains_texture(t.texture_descriptor.color)
            ? secondary_rendering_resources_.texture_target(t.texture_descriptor.color, TextureRole::COLOR_FROM_DB)
            : primary_rendering_resources_.texture_target(t.texture_descriptor.color, TextureRole::COLOR_FROM_DB);
        if (ttype != TextureTarget::TEXTURE_2D_ARRAY) {
            throw std::runtime_error("Unexpected texture type (expected a 2D array)");
        }
    }
    if (vertices->has_discrete_triangle_texture_layers() &&
        has_discrete_atlas_texture_layer)
    {
        throw std::runtime_error("Detected discrete texture layer per vertex and per instance");
    }
    TextureLayerProperties texture_layer_properties = TextureLayerProperties::NONE;
    if (vertices->has_discrete_triangle_texture_layers()) {
        add(texture_layer_properties, (
            TextureLayerProperties::DISCRETE |
            TextureLayerProperties::VERTEX));
    }
    if (has_discrete_atlas_texture_layer) {
        add(texture_layer_properties, (
            TextureLayerProperties::DISCRETE |
            TextureLayerProperties::ATLAS));
    }
    if (vertices->has_continuous_triangle_texture_layers()) {
        add(texture_layer_properties, (
            TextureLayerProperties::CONTINUOUS |
            TextureLayerProperties::VERTEX));
    }
    if ((instances != nullptr) && instances->has_continuous_texture_layer()) {
        if (any(texture_layer_properties)) {
            throw std::runtime_error("Detected continuous texture layers in both, vertices and instances");
        }
        add(texture_layer_properties, (
            TextureLayerProperties::CONTINUOUS |
            TextureLayerProperties::VERTEX));
    }
    if (!std::isnan(texture_layer)) {
        if (any(texture_layer_properties)) {
            throw std::runtime_error("Detected continuous texture layers in both, renderable and animation");
        }
        add(texture_layer_properties, (
            TextureLayerProperties::CONTINUOUS |
            TextureLayerProperties::UNIFORM));
    }
    auto attr_idc = get_attribute_index_calculator(*cva);
    const ColoredRenderProgram& rp = get_render_program(
        RenderProgramIdentifier{
            .attr_idc = attr_idc,
            .render_pass = render_pass.rsd.external_render_pass.pass,
            .skidmarks_hash = skidmarks_hash,
            .nbones = (acvas == nullptr) ? 0 : acvas->bone_indices.size(),
            .blend_mode = any(render_pass.rsd.external_render_pass.pass & ExternalRenderPassType::LIGHTMAP_BLOBS_MASK)
                ? BlendMode::CONTINUOUS
                : meta.material.blend_mode,
            .alpha_distances = alpha_distances_common,
            .fog_distances = fog_distances,
            .fog_emissive = make_orderable(fog_emissive),
            .has_normalmap = has_normalmap(meta.material.textures_color),
            .ntextures_color = texture_ids_color.size(),
            .ntextures_normal = texture_ids_normal.size(),
            .ntextures_alpha = texture_ids_alpha.size(),
            .has_dynamic_emissive = has_dynamic_emissive,
            .lightmap_indices_color = lightmap_indices_color,
            .lightmap_indices_depth = lightmap_indices_depth,
            .has_specularmap = !texture_ids_specular.empty(),
            .reflectance = make_orderable(reflectance),
            .reflect_only_y = meta.material.reflect_only_y,
            .ntextures_reflection = ntextures_reflection,
            .ntextures_dirt = ntextures_dirt,
            .interior_texture_set = interior_texture_set,
            .facade_inner_size = meta.material.interior_textures.facade_inner_size,
            .interior_size = meta.material.interior_textures.interior_size,
            .nuv_indices = is_lightmap ? 1 : vertices->nuvs(),
            .ncweights = is_lightmap ? 0 : vertices->ncweights(),
            .has_alpha = vertices->has_alpha(),
            .continuous_layer_x = meta.material.continuous_layer_x,
            .continuous_layer_y = meta.material.continuous_layer_y,
            .has_horizontal_detailmap = has_horizontal_detailmap,
            .dirt_color_mode = (ntextures_dirt != 0)
                ? primary_rendering_resources_.get_texture_descriptor(meta.material.dirt_texture.variable_and_hash()).color.color_mode
                : ColorMode::UNDEFINED,
            .has_instances = has_instances,
            .has_flat = has_flat,
            .has_lookat = has_lookat,
            .has_yangle = has_yangle,
            .has_rotation_quaternion = has_rotation_quaternion,
            .has_uv_offset_u = (meta.material.number_of_frames != 1),  // Texture is required in lightmap also due to alpha channel.
            .texture_layer_properties = texture_layer_properties,
            .nbillboard_ids = integral_cast<BillboardId>(meta.material.billboard_atlas_instances.size()),  // Texture is required in lightmap also due to alpha channel.
            .reorient_normals = reorient_normals,
            .reorient_uv0 = reorient_uv0,
            .emissive = make_orderable(emissive),
            .ambient = make_orderable(ambient),
            .diffuse = make_orderable(diffuse),
            .specular = make_orderable(specular),
            .specular_exponent = specular_exponent,
            .fresnel_emissive = make_orderable(fresnel_emissive),
            .fresnel = fresnel,
            .alpha = meta.material.alpha,
            .orthographic = vc.orthographic(),
            .fragments_depend_on_distance = fragments_depend_on_distance,
            .fragments_depend_on_normal = fragments_depend_on_normal,
            // Not using NAN for ordering.
            .dirtmap_offset = (ntextures_dirt != 0) ? secondary_rendering_resources_.get_offset(dirtmap_name) : -1234,
            .dirtmap_discreteness = (ntextures_dirt != 0) ? secondary_rendering_resources_.get_discreteness(dirtmap_name) : -1234,
            .dirt_scale = (ntextures_dirt != 0) ? secondary_rendering_resources_.get_scale(dirtmap_name) : -1234,
            .texture_modifiers_hash = texture_modifiers_hash,
            .lights_hash = lights_hash},
        *cva,
        acvas,
        filtered_lights,
        filtered_skidmarks,
        lightmap_indices,
        light_noshadow_indices,
        light_shadow_indices,
        black_shadow_indices,
        blended_textures_color,
        blended_textures_alpha);
    {
        static const char* json_filename = getenv("JSON_RENDER_CONFIG");
        if (json_filename != nullptr) {
            JsonObjectFile j;
            try {
                j.load_from_file(json_filename);
                if (j.at<bool>("print_shaders")) {
                    linfo() << "Render pass: " << external_render_pass_type_to_string(render_pass.rsd.external_render_pass.pass);
                    linfo() << "Attribute index calculator";
                    linfo() << attr_idc;
                    linfo() << "Vertex array";
                    cva->print_stats(linfo().ref());
                    linfo() << "Vertex shader";
                    linfo() << rp.vertex_shader_text;
                    linfo() << "Fragment shader";
                    linfo() << rp.fragment_shader_text;
                }
            } catch (std::exception& e) {
                lwarn() << e.what();
            }
        }
    }
    LOG_INFO("RenderableColoredVertexArray::render_cva glUseProgram");
    rp.use();
    LOG_INFO("RenderableColoredVertexArray::render_cva mvp");
    CHK(glUniformMatrix4fv(rp.mvp_location, 1, GL_TRUE, mvp.casted<float>().flat_begin()));
    if (meta.material.number_of_frames != 1) {
        float uv_offset_u;
        if ((animation_state != nullptr) &&
            !animation_state->aperiodic_animation_frame.is_nan())
        {
            float duration = animation_state->aperiodic_animation_frame.duration();
            if (duration == 0.f) {
                uv_offset_u = animation_state->aperiodic_animation_frame.time();
            } else {
                float frame_index = std::floor(frame_index_from_animation_state(
                    animation_state->aperiodic_animation_frame.elapsed(),
                    duration,
                    meta.material.number_of_frames));
                uv_offset_u = frame_index / (float)meta.material.number_of_frames;
            }
        } else {
            throw std::runtime_error(
                "Material has multiple frames, but animation state "
                "is not set or not active: " + meta.name.full_name() + ", " + meta.material.identifier());
            // uv_offset_u = 0;
        }
        CHK(glUniform1f(rp.uv_offset_u_location, uv_offset_u));
    }
    if (!std::isnan(texture_layer)) {
        CHK(glUniform1f(
            rp.texture_layer_location_uniform,
            texture_layer));
    }
    if (!meta.material.billboard_atlas_instances.empty()) {
        size_t n = meta.material.billboard_atlas_instances.size();
        auto ni = integral_cast<GLsizei>(n);
        UUVector<FixedArray<float, 3>> vertex_scale(n);
        UUVector<FixedArray<float, 2>> uv_scale(n);
        UUVector<FixedArray<float, 2>> uv_offset(n);
        std::vector<GLuint> texture_layers;
        UUVector<FixedArray<float, 4>> alpha_distances_billboards;
        if (has_discrete_atlas_texture_layer) {
            texture_layers.resize(n);
        }
        if (!vc.orthographic()) {
            alpha_distances_billboards.resize(n);
        }
        for (size_t i = 0; i < n; ++i) {
            uv_offset[i] = meta.material.billboard_atlas_instances[i].uv_offset;
            uv_scale[i] = meta.material.billboard_atlas_instances[i].uv_scale;
            vertex_scale[i] = meta.material.billboard_atlas_instances[i].vertex_scale;
            if (has_discrete_atlas_texture_layer) {
                texture_layers[i] = integral_cast<GLuint>(meta.material.billboard_atlas_instances[i].texture_layer);
            }
            if (!vc.orthographic()) {
                alpha_distances_billboards[i] = meta.material.billboard_atlas_instances[i].alpha_distances;
            }
        }
        CHK(glUniform2fv(rp.uv_offset_location, ni, (const GLfloat*)uv_offset.data()));
        CHK(glUniform2fv(rp.uv_scale_location, ni, (const GLfloat*)uv_scale.data()));
        CHK(glUniform3fv(rp.vertex_scale_location, ni, (const GLfloat*)vertex_scale.data()));
        if (has_discrete_atlas_texture_layer) {
            CHK(glUniform1uiv(rp.texture_layers_location_atlas, ni, (const GLuint*)texture_layers.data()));
        }
        if (!vc.orthographic()) {
            CHK(glUniform4fv(rp.alpha_distances_location, ni, (const GLfloat*)alpha_distances_billboards.data()));
        }
    }
    if (has_dynamic_emissive) {
        FixedArray<float, 3> dynamic_emissive = dynamic_style != nullptr
            ? dynamic_style->emissive
            : fixed_zeros<float, 3>();
        CHK(glUniform3fv(rp.dynamic_emissive_location, 1, (const GLfloat*)&dynamic_emissive));
    }
    LOG_INFO("RenderableColoredVertexArray::render_cva lights");
    {
        bool light_dir_required = (any(diffuse != 0.f) || any(specular != 0.f));
        if (light_dir_required || fragments_depend_on_distance || fragments_depend_on_normal || (ntextures_interior != 0)) {
            // CHK(glUniform3fv(rp.light_position_location, 1, t3_from_4x4(filtered_lights.front().first).flat_begin()));
            if (light_dir_required) {
                size_t i = 0;
                for (const auto& [trafo, light] : filtered_lights) {
                    if (!any(light->shadow_render_pass & ExternalRenderPassType::LIGHTMAP_IS_BLACK_MASK)) {
                        auto mz = m.irotate(z3_from_3x3(trafo.R));
                        mz /= std::sqrt(sum(squared(mz)));
                        CHK(glUniform3fv(rp.light_dir_locations.at(i), 1, mz.flat_begin()));
                    }
                    ++i;
                }
            }
        }
    }
    {
        size_t i = 0;
        for (const auto& [_, light] : filtered_lights) {
            if (any(ambient != 0.f) &&
                any(light->ambient != 0.f) &&
                !any(light->shadow_render_pass & ExternalRenderPassType::LIGHTMAP_IS_BLACK_MASK))
            {
                CHK(glUniform3fv(rp.light_ambients.at(i), 1, light->ambient.flat_begin()));
            }
            if (any(diffuse != 0.f) &&
                any(light->diffuse != 0.f) &&
                !any(light->shadow_render_pass & ExternalRenderPassType::LIGHTMAP_IS_BLACK_MASK))
            {
                CHK(glUniform3fv(rp.light_diffuses.at(i), 1, light->diffuse.flat_begin()));
            }
            if (any(specular != 0.f) &&
                any(light->specular != 0.f) &&
                !any(light->shadow_render_pass & ExternalRenderPassType::LIGHTMAP_IS_BLACK_MASK))
            {
                CHK(glUniform3fv(rp.light_speculars.at(i), 1, light->specular.flat_begin()));
            }
            ++i;
        }
    }
    {
        bool pred0 =
            has_lookat ||
            (any(specular != 0.f) && (specular_exponent != 0.f)) ||
            any(reflectance != 0.f) ||
            any(interior_texture_set & InteriorTextureSet::ANY_SPECULAR) ||
            (fragments_depend_on_distance && !vc.orthographic()) ||
            (fresnel.exponent != 0.f);
        bool pred1 = (fog_distances != default_step_distances);
        if (has_flat || pred0 || pred1 || reorient_uv0 || (ntextures_interior != 0) || reorient_normals) {
            bool ortho = vc.orthographic();
            auto miv = m.inverted() * iv;
            if ((pred0 || pred1 || reorient_uv0 || reorient_normals) && ortho) {
                auto d = z3_from_3x3(miv.R);
                d /= std::sqrt(sum(squared(d)));
                CHK(glUniform3fv(rp.view_dir, 1, d.flat_begin()));
            }
            if ((pred0 && !ortho) || (ntextures_interior != 0) || pred1) {
                CHK(glUniform3fv(rp.view_pos, 1, miv.t.casted<float>().flat_begin()));
            }
            if (has_flat) {
                CHK(glUniformMatrix3fv(rp.lookat_location, 1, GL_TRUE, miv.R.flat_begin()));
            }
        }
    }
    if (any(reflectance != 0.f) || any(interior_texture_set & InteriorTextureSet::ANY_SPECULAR)) {
        CHK(glUniformMatrix3fv(rp.r_location, 1, GL_TRUE, m.R.flat_begin()));
    }
    if ((acvas != nullptr) && !acvas->bone_indices.empty()) {
        for (const auto& [i, l] : enumerate(absolute_bone_transformations)) {
            CHK(glUniform3fv(rp.pose_positions.at(i), 1, l.t.flat_begin()));
            CHK(glUniform4fv(rp.pose_quaternions.at(i), 1, l.q.v.flat_begin()));
        }
    }
    if (has_horizontal_detailmap) {
        if (meta.material.period_world == 0.f) {
            throw std::runtime_error("Horizontal detailmap requires world period");
        }
        // Moving nodes should use the uv1, uv2, ... fields for detailmaps.
        // if (render_pass.internal != InternalRenderPass::AGGREGATE) {
        //     throw std::runtime_error("Horizontal detailmap requires aggregation");
        // }
        FixedArray<float, 2> rem{
            (float)std::remainder(m.t(0), meta.material.period_world),
            (float)std::remainder(m.t(2), meta.material.period_world)};
        CHK(glUniform2fv(rp.horizontal_detailmap_remainder, 1, rem.flat_begin()));
    }
    LOG_INFO("RenderableColoredVertexArray::render_cva bind texture");
    TextureBinder tb;
    for (const auto& [c, i] : texture_ids_color) {
        LOG_INFO("RenderableColoredVertexArray::render_cva clamp texture \"" + c->filename + '"');
        try {
            tb.bind(rp.texture_color_locations.at(i.id), i.texture, InterpolationPolicy::AUTO, texture_layer_properties);
        } catch (const std::runtime_error& e) {
            throw std::runtime_error("Texture \"" + c->filename.string() + "\": " + e.what());
        }
    }
    for (const auto& [c, i] : texture_ids_alpha) {
        LOG_INFO("RenderableColoredVertexArray::render_cva clamp texture \"" + c->filename + '"');
        tb.bind(rp.texture_alpha_locations.at(i.id), i.texture, InterpolationPolicy::AUTO, TextureLayerProperties::NONE);
    }
    assert_true(lightmap_indices_color.empty() || lightmap_indices_depth.empty());
    LOG_INFO("RenderableColoredVertexArray::render_cva bind light color textures");
    if (!lightmap_indices_color.empty()) {
        for (auto l : lightmap_indices) {
            const auto& light = *filtered_lights.at(l).second;
            if (!light.vp.has_value()) {
                throw std::runtime_error("Lightmap has no VP");
            }
            if (light.lightmap_color == nullptr) {
                throw std::runtime_error("Lightmap has no color texture");
            }
            auto mvp_light = dot2d(*light.vp, m.affine());
            CHK(glUniformMatrix4fv(rp.mvp_light_locations.at(l), 1, GL_TRUE, mvp_light.casted<float>().flat_begin()));

            tb.bind(rp.texture_lightmap_color_locations.at(l), *light.lightmap_color, InterpolationPolicy::AUTO, TextureLayerProperties::NONE);
        }
    }
    LOG_INFO("RenderableColoredVertexArray::render_cva bind light depth textures");
    if (!lightmap_indices_depth.empty()) {
        for (auto l : lightmap_indices) {
            const auto& light = *filtered_lights.at(l).second;
            if (!light.vp.has_value()) {
                throw std::runtime_error("Lightmap has no VP");
            }
            if (light.lightmap_depth == nullptr) {
                throw std::runtime_error("Lightmap has no depth texture");
            }
            auto mvp_light = dot2d(*light.vp, m.affine());
            CHK(glUniformMatrix4fv(rp.mvp_light_locations.at(l), 1, GL_TRUE, mvp_light.casted<float>().flat_begin()));

            tb.bind(rp.texture_lightmap_depth_locations.at(l), *light.lightmap_depth, InterpolationPolicy::AUTO, TextureLayerProperties::NONE);
        }
    }
    LOG_INFO("RenderableColoredVertexArray::render_cva bind normalmap texture");
    for (const auto& [c, i] : texture_ids_normal) {
        assert_true(!c->filename.empty());
        tb.bind(rp.texture_normalmap_locations.at(i.id), i.texture, InterpolationPolicy::AUTO, texture_layer_properties);
    }
    LOG_INFO("RenderableColoredVertexArray::render_cva bind skidmark texture");
    for (const auto& [i, s] : enumerate(filtered_skidmarks)) {
        const auto& skidmark = *s.second;
        if (skidmark.texture == nullptr) {
            throw std::runtime_error("Skidmark has no texture");
        }
        auto mvp_skidmark = dot2d(skidmark.vp, m.affine());
        CHK(glUniformMatrix4fv(rp.mvp_skidmarks_locations.at(i), 1, GL_TRUE, mvp_skidmark.casted<float>().flat_begin()));

        tb.bind(rp.texture_skidmark_locations.at(i), *skidmark.texture, InterpolationPolicy::AUTO, TextureLayerProperties::NONE);
    }
    LOG_INFO("RenderableColoredVertexArray::render_cva bind reflection texture");
    if (ntextures_reflection != 0) {
        assert_true(reflection_map != nullptr);
        const auto& texture = *primary_rendering_resources_.get_texture(*reflection_map);
        tb.bind(rp.texture_reflection_location, texture, InterpolationPolicy::AUTO, TextureLayerProperties::NONE);
    }
    LOG_INFO("RenderableColoredVertexArray::render_cva bind dirtmap texture");
    if (ntextures_dirt != 0) {
        assert_true(ntextures_dirt == 2);
        const auto& mname = dirtmap_name;
        {
            const auto& dirtmap_vp = secondary_rendering_resources_.get_vp(mname);
            auto mvp_dirtmap = dot2d(dirtmap_vp, m.affine());
            CHK(glUniformMatrix4fv(rp.mvp_dirtmap_location, 1, GL_TRUE, mvp_dirtmap.casted<float>().flat_begin()));
        }

        {
            auto dname = secondary_rendering_resources_.get_alias(mname);
            const auto& texture = *primary_rendering_resources_.get_texture(dname);
            if (texture.color_mode() != ColorMode::GRAYSCALE) {
                throw std::runtime_error("Dirtmap \"" + dname.string() + "\" does not have colormode grayscale");
            }
            tb.bind(rp.texture_dirtmap_location, texture, InterpolationPolicy::AUTO, TextureLayerProperties::NONE);
        }

        {
            const auto& texture = *primary_rendering_resources_.get_texture(meta.material.dirt_texture);
            if ((texture.color_mode() != ColorMode::RGB) &&
                (texture.color_mode() != ColorMode::RGBA))
            {
                throw std::runtime_error("Dirt texture \"" + meta.material.dirt_texture.string() + "\" does not have colormode RGB or RGBA");
            }
            tb.bind(rp.texture_dirt_location, texture, InterpolationPolicy::AUTO, TextureLayerProperties::NONE);
        }
    }
    if (any(interior_texture_set)) {
        for (size_t i = 0; i < size(InteriorTextureSet::INTERIOR_COLORS); ++i) {
            const auto& h = *primary_rendering_resources_.get_texture(meta.material.interior_textures[i], TextureRole::COLOR_FROM_DB);
            if (h.color_mode() != ColorMode::RGB) {
                throw std::runtime_error("Interiormap color texture \"" + meta.material.interior_textures[i].string() + "\" does not have color mode RGB");
            }
            tb.bind(rp.texture_interiormap_location(i), h, InterpolationPolicy::AUTO, TextureLayerProperties::NONE);
        }
    }
    if (any(interior_texture_set & InteriorTextureSet::BACK_SPECULAR)) {
        auto i = index(interior_texture_set, InteriorTextureSet::BACK_SPECULAR);
        const auto& h = *primary_rendering_resources_.get_texture(meta.material.interior_textures[i], TextureRole::SPECULAR);
        if (h.color_mode() != ColorMode::RGB) {
            throw std::runtime_error("Interiormap back specular texture \"" + meta.material.interior_textures[i].string() + "\" does not have color mode RGB");
        }
        tb.bind(rp.texture_interiormap_location(i), h, InterpolationPolicy::AUTO, TextureLayerProperties::NONE);
    }
    if (any(interior_texture_set & InteriorTextureSet::FRONT_COLOR)) {
        auto i = index(interior_texture_set, InteriorTextureSet::FRONT_COLOR);
        const auto& h = *primary_rendering_resources_.get_texture(meta.material.interior_textures[i], TextureRole::SPECULAR);
        if (h.color_mode() != ColorMode::RGB) {
            throw std::runtime_error("Interiormap front color texture \"" + meta.material.interior_textures[i].string() + "\" does not have color mode RGB");
        }
        tb.bind(rp.texture_interiormap_location(i), h, InterpolationPolicy::AUTO, TextureLayerProperties::NONE);
    }
    if (any(interior_texture_set & InteriorTextureSet::FRONT_ALPHA)) {
        auto i = index(interior_texture_set, InteriorTextureSet::FRONT_ALPHA);
        const auto& h = *primary_rendering_resources_.get_texture(meta.material.interior_textures[i], TextureRole::COLOR);
        if (h.color_mode() != ColorMode::GRAYSCALE) {
            throw std::runtime_error("Interiormap front alpha texture \"" + meta.material.interior_textures[i].string() + "\" does not have color mode R");
        }
        tb.bind(rp.texture_interiormap_location(i), h, InterpolationPolicy::AUTO, TextureLayerProperties::NONE);
    }
    if (any(interior_texture_set & InteriorTextureSet::FRONT_SPECULAR)) {
        auto i = index(interior_texture_set, InteriorTextureSet::FRONT_SPECULAR);
        const auto& h = *primary_rendering_resources_.get_texture(meta.material.interior_textures[i], TextureRole::SPECULAR);
        if (h.color_mode() != ColorMode::RGB) {
            throw std::runtime_error("Interiormap front specular texture \"" + meta.material.interior_textures[i].string() + "\" does not have color mode RGB");
        }
        tb.bind(rp.texture_interiormap_location(i), h, InterpolationPolicy::AUTO, TextureLayerProperties::NONE);
    }
    if (texture_ids_specular.size() != 0) {
        assert_true(texture_ids_specular.size() == 1);
        assert_true(!meta.material.textures_color.empty());
        const auto& desc = meta.material.textures_color[0].texture_descriptor;
        assert_true(!desc.specular.filename.empty());
        const auto& texture = *primary_rendering_resources_.get_texture(desc.specular);
        tb.bind(rp.texture_specularmap_location, texture, InterpolationPolicy::AUTO, TextureLayerProperties::NONE);
    }
    if ((render_pass.rsd.external_render_pass.pass != ExternalRenderPassType::DIRTMAP) &&
        !is_lightmap &&
        (meta.material.draw_distance_noperations > 0) &&
        (
            std::isnan(render_config.draw_distance_add) ||
            (render_config.draw_distance_add != INFINITY)))
    {
        if ((acvas != nullptr) && !acvas->bone_indices.empty()) {
            throw std::runtime_error("Draw distance incompatible with animations");
        }
        // This is legacy code kept in case it finds a new use case.
        // As of now, deleting triangles far away is done during
        // the aggregation step, which also converts double to float,
        // making the following code obsolete.
        vertices->delete_triangles_far_away_legacy(
            iv.t.casted<float>(),
            m.casted<float, float>(),
            std::isnan(render_config.draw_distance_add)
                ? meta.material.draw_distance_add
                : render_config.draw_distance_add,
            std::isnan(render_config.draw_distance_slop)
                ? meta.material.draw_distance_slop
                : render_config.draw_distance_slop,
            meta.material.draw_distance_noperations,
            true,  // run_in_background
            true); // is_static
    }
    LOG_INFO("RenderableColoredVertexArray::render_cva glBindVertexArray");
    {
        // AperiodicLagFinder lag_finder{ "draw " + meta.name + ": ", std::chrono::milliseconds{5} };
        MaterialRenderConfigGuard mrcf{ meta.material, render_pass.internal };
        if (has_instances) {
            if (any(render_pass.internal & InternalRenderPass::PRELOADED) &&
                instances->copy_in_progress())
            {
                cva->print_stats(lerr().ref());
                verbose_abort("Preloaded render pass has incomplete instances: \"" + meta.name.full_name() + '"');
            }
            instances->wait();
        }
        if (any(render_pass.internal & InternalRenderPass::PRELOADED) &&
            cva->copy_in_progress())
        {
            cva->print_stats(lerr().ref());
            verbose_abort("Preloaded render pass has incomplete triangles: \"" + meta.name.full_name() + '"');
        }
        if (!cva->initialized()) {
            cva->initialize();
        }
        cva->update_legacy();
        cva->bind();
        LOG_INFO("RenderableColoredVertexArray::render_cva glDrawArrays");
        if (has_instances) {
            try {
                notify_rendering(CURRENT_SOURCE_LOCATION);
                TemporarilyIgnoreFloatingPointExeptions ignore_except;
                CHK(glDrawArraysInstanced(GL_TRIANGLES, 0, integral_cast<GLsizei>(3 * vertices->ntriangles()), integral_cast<GLsizei>(instances->num_instances())));
            } catch (const std::runtime_error& e) {
                cva->print_stats(lerr(LogFlags::NO_APPEND_NEWLINE).ref());
                throw std::runtime_error(
                    (std::stringstream() <<
                    "Could not render instanced triangles: " << e.what() << '\n' <<
                    "  #triangles: " << vertices->ntriangles() << '\n' <<
                    "  #instances: " << instances->num_instances() << '\n' <<
                    "  name: " << meta.name << '\n' <<
                    "  material: " << meta.material.identifier() << '\n' <<
                    "  physics material: " << physics_material_to_string(meta.morphology.physics_material)).str());
            }
        } else {
            try {
                notify_rendering(CURRENT_SOURCE_LOCATION);
                TemporarilyIgnoreFloatingPointExeptions ignore_except;
                CHK(glDrawArrays(GL_TRIANGLES, 0, integral_cast<GLsizei>(3 * vertices->ntriangles())));
            } catch (const std::runtime_error& e) {
                cva->print_stats(lerr(LogFlags::NO_APPEND_NEWLINE).ref());
                throw std::runtime_error(
                    (std::stringstream() <<
                    "Could not render triangles: " << e.what() << '\n' <<
                    "  #triangles: " << vertices->ntriangles() << '\n' <<
                    "  name: " << meta.name << '\n' <<
                    "  material: " << meta.material.identifier() << '\n' <<
                    "  physics material: " << physics_material_to_string(meta.morphology.physics_material)).str());
            }
        }
        CHK(glBindVertexArray(0));
    }
    // CHK(glFlush());
    LOG_INFO("RenderableColoredVertexArray::render_cva glDrawArrays finished");
}

const ColoredRenderProgram& OpenGLVertexArrayRenderer::get_render_program(
    const RenderProgramIdentifier& id,
    const IGpuVertexArray& gva,
    const std::shared_ptr<AnimatedColoredVertexArrays>& acvas,
    const std::vector<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<Light>>>& filtered_lights,
    const std::vector<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<Skidmark>>>& filtered_skidmarks,
    const std::vector<size_t>& lightmap_indices,
    const std::vector<size_t>& light_noshadow_indices,
    const std::vector<size_t>& light_shadow_indices,
    const std::vector<size_t>& black_shadow_indices,
    const std::vector<BlendMapTextureAndId>& textures_color,
    const std::vector<BlendMapTextureAndId>& textures_alpha) const
{
    auto rps = primary_rendering_resources_.render_programs();
    if (auto it = rps.try_get(id); it != nullptr) {
        return **it;
    }
    if (!(id == id)) {
        throw std::runtime_error("Render program identifier contains NAN values");
    }
    if (acvas != nullptr) {
        assert_true(acvas->bone_indices.empty() == !acvas->skeleton);
    }
    auto attr_ids = id.attr_idc.build();
    OrderedStandardMap<UvMapKey, size_t> uv_map;
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
        NotSortedStruct{ id.render_pass },
        NotSortedStruct{ id.attr_idc },
        attr_ids,
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
        id.has_flat,
        id.has_lookat,
        id.has_yangle,
        id.has_rotation_quaternion,
        id.has_uv_offset_u,
        (acvas == nullptr) ? 0 : acvas->bone_indices.size(),
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
        any(id.blend_mode & BlendMode::ANY_CONTINUOUS),
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
    auto rp = std::make_unique<ColoredRenderProgram>();
    try {
        rp->vertex_array_identifier = gva.identifier();

        rp->allocate(vs_text, fs_text);

        rp->vertex_shader_text = vs_text;
        rp->fragment_shader_text = fs_text;

        rp->mvp_location = rp->get_uniform_location("MVP");
        if (id.has_flat) {
            rp->lookat_location = rp->get_uniform_location("lookat");
        } else {
            rp->lookat_location = -1;
        }
        if (id.has_uv_offset_u) {
            rp->uv_offset_u_location = rp->get_uniform_location("uv_offset_u");
        } else {
            rp->uv_offset_u_location = -1;
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
            // rp->mvp_light_location = -1;
        }
        for (size_t i = 0; i < filtered_skidmarks.size(); ++i) {
            rp->mvp_skidmarks_locations[i] = rp->get_uniform_location(("MVP_skidmarks[" + std::to_string(i) + "]").c_str());
        }
        if (any(id.texture_layer_properties & TextureLayerProperties::UNIFORM)) {
            rp->texture_layer_location_uniform = rp->get_uniform_location("texture_layer_fs");
        } else {
            rp->texture_layer_location_uniform = -1;
        }
        if (id.nbillboard_ids != 0) {
            rp->vertex_scale_location = rp->get_uniform_location("vertex_scale");
            rp->uv_scale_location = rp->get_uniform_location("uv_scale");
            rp->uv_offset_location = rp->get_uniform_location("uv_offset");
            if (any(id.texture_layer_properties & TextureLayerProperties::ATLAS)) {
                rp->texture_layers_location_atlas = rp->get_uniform_location("texture_layers");
            } else {
                rp->texture_layers_location_atlas = -1;
            }
            if (!id.orthographic) {
                rp->alpha_distances_location = rp->get_uniform_location("alpha_distances");
            } else {
                rp->alpha_distances_location = -1;
            }
        } else {
            rp->vertex_scale_location = -1;
            rp->uv_scale_location = -1;
            rp->uv_offset_location = -1;
            rp->texture_layers_location_atlas = -1;
            rp->alpha_distances_location = -1;
        }
        if (id.has_dynamic_emissive) {
            rp->dynamic_emissive_location = rp->get_uniform_location("dynamic_emissive");
        } else {
            rp->dynamic_emissive_location = -1;
        }
        assert(id.lightmap_indices_color.empty() || id.lightmap_indices_depth.empty());
        if (!id.lightmap_indices_color.empty()) {
            for (size_t i : lightmap_indices) {
                rp->texture_lightmap_color_locations[i] = rp->get_uniform_location(("texture_light_color" + std::to_string(i)).c_str());
            }
        } else {
            // Do nothing
            // rp->texture_lightmap_color_location = -1;
        }
        if (!id.lightmap_indices_depth.empty()) {
            for (size_t i : lightmap_indices) {
                rp->texture_lightmap_depth_locations[i] = rp->get_uniform_location(("texture_light_depth" + std::to_string(i)).c_str());
            }
        } else {
            // Do nothing
            // rp->texture_lightmap_depth_location = -1;
        }
        if (id.ntextures_normal != 0) {
            for (const auto& r : textures_color) {
                if (!r->texture_descriptor.normal.filename.empty()) {
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
            rp->texture_reflection_location = -1;
        }
        if (id.ntextures_dirt != 0) {
            rp->mvp_dirtmap_location = rp->get_uniform_location("MVP_dirtmap");
            rp->texture_dirtmap_location = rp->get_uniform_location("texture_dirtmap");
            rp->texture_dirt_location = rp->get_uniform_location("texture_dirt");
        } else {
            rp->mvp_dirtmap_location = -1;
            rp->texture_dirtmap_location = -1;
            rp->texture_dirt_location = -1;
        }
        if (any(id.interior_texture_set)) {
            auto n = size(id.interior_texture_set);
            for (size_t i = 0; i < n; ++i) {
                rp->texture_interiormap_location(i) = rp->get_uniform_location(("texture_interior[" + std::to_string(i) + "]").c_str());
            }
        } else {
            rp->texture_interiormap_location = -1;
        }
        if (id.has_specularmap) {
            rp->texture_specularmap_location = rp->get_uniform_location("texture_specularmap");
        } else {
            rp->texture_specularmap_location = -1;
        }
        if (!id.reflectance.all_equal(0.f) || any(id.interior_texture_set & InteriorTextureSet::ANY_SPECULAR)) {
            rp->r_location = rp->get_uniform_location("R");
        } else {
            rp->r_location = -1;
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
        if (acvas != nullptr) {
            assert_true(acvas->bone_indices.empty() == !acvas->skeleton);
            for (size_t i = 0; i < acvas->bone_indices.size(); ++i) {
                rp->pose_positions[i] = rp->get_uniform_location(("bone_positions[" + std::to_string(i) + "]").c_str());
                rp->pose_quaternions[i] = rp->get_uniform_location(("bone_quaternions[" + std::to_string(i) + "]").c_str());
            }
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

        return *rps.add(id, std::move(rp));
    } catch (const std::runtime_error& e) {
        std::string identifier;
        if (!textures_color.empty()) {
            identifier = "\nAmbient+diffuse: " + textures_color[0]->texture_descriptor.color.filename.string();
        }
        throw std::runtime_error(
            std::string("Could not generate render program.\n") +
            e.what() +
            identifier +
            "\nVertex shader:\n" + vs_text +
            "\nFragment shader:\n" + fs_text);
    }
}
