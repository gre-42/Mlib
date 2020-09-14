#include "Renderable_Colored_Vertex_Array.hpp"
#include <Mlib/Geometry/Mesh/Triangle_Rays.hpp>
#include <Mlib/Images/Coordinates_Fixed.hpp>
#include <Mlib/Images/Revert_Axis.hpp>
#include <Mlib/Images/Vectorial_Pixels.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Gen_Shader_Text.hpp>
#include <Mlib/Render/Renderables/Renderable_Colored_Vertex_Array_Instance.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>
#include <Mlib/Scene_Graph/Scene_Node.hpp>
#include <iostream>

using namespace Mlib;

static GenShaderText vertex_shader_text_gen{[](
    const std::list<std::pair<FixedArray<float, 4, 4>, Light*>>& lights,
    size_t nlights,
    bool has_lightmap_color,
    bool has_lightmap_depth,
    bool has_dirtmap,
    bool has_diffusivity,
    bool has_specularity,
    bool reorient_normals)
{
    assert_true(nlights == lights.size());
    std::stringstream sstr;
    sstr << "#version 330 core" << std::endl;
    sstr << "uniform mat4 MVP;" << std::endl;
    if (has_diffusivity || has_specularity) {
        sstr << "uniform mat4 M;" << std::endl;
    }
    sstr << "layout (location=0) in vec3 vPos;" << std::endl;
    sstr << "layout (location=1) in vec3 vCol;" << std::endl;
    sstr << "layout (location=2) in vec2 vTexCoord;" << std::endl;
    if (has_diffusivity || has_specularity) {
        sstr << "layout (location=3) in vec3 vNormal;" << std::endl;
    }
    sstr << "out vec3 color;" << std::endl;
    sstr << "out vec2 tex_coord;" << std::endl;
    if (has_lightmap_color || has_lightmap_depth) {
        sstr << "uniform mat4 MVP_light[" << lights.size() << "];" << std::endl;
        // vec4 to avoid clipping problems
        sstr << "out vec4 FragPosLightSpace[" << lights.size() << "];" << std::endl;
    }
    if (has_dirtmap) {
        sstr << "uniform mat4 MVP_dirtmap;" << std::endl;
        sstr << "out vec2 tex_coord_dirtmap;" << std::endl;
    }
    if (reorient_normals || has_specularity) {
        sstr << "out vec3 FragPos;" << std::endl;
    }
    if (has_diffusivity || has_specularity) {
        sstr << "out vec3 Normal;" << std::endl;
    }
    sstr << "void main()" << std::endl;
    sstr << "{" << std::endl;
    sstr << "    gl_Position = MVP * vec4(vPos, 1.0);" << std::endl;
    sstr << "    color = vCol;" << std::endl;
    sstr << "    tex_coord = vTexCoord;" << std::endl;
    if (has_lightmap_color || has_lightmap_depth) {
        sstr << "    for (int i = 0; i < " << lights.size() << "; ++i) {" << std::endl;
        sstr << "        FragPosLightSpace[i] = MVP_light[i] * vec4(vPos, 1.0);" << std::endl;
        sstr << "    }" << std::endl;
    }
    if (has_dirtmap) {
        sstr << "    vec4 pos4_dirtmap = MVP_dirtmap * vec4(vPos, 1.0);" << std::endl;
        sstr << "    tex_coord_dirtmap = (pos4_dirtmap.xy / pos4_dirtmap.w + 1) / 2;" << std::endl;
    }
    if (reorient_normals || has_specularity) {
        sstr << "    FragPos = vec3(M * vec4(vPos, 1.0));" << std::endl;
    }
    if (has_diffusivity || has_specularity) {
        sstr << "    Normal = mat3(M) * vNormal;" << std::endl;
    }
    sstr << "}" << std::endl;
    return sstr.str();
}};

enum class OcclusionType {
    OFF,
    OCCLUDED,
    OCCLUDER
};

static GenShaderText fragment_shader_text_textured_rgb_gen{[](
    const std::list<std::pair<FixedArray<float, 4, 4>, Light*>>& lights,
    size_t nlights,
    bool has_texture,
    bool has_lightmap_color,
    bool has_lightmap_depth,
    bool has_dirtmap,
    const OrderableFixedArray<float, 3>& ambience,
    const OrderableFixedArray<float, 3>& diffusivity,
    const OrderableFixedArray<float, 3>& specularity,
    float alpha_threshold,
    OcclusionType occlusion_type,
    bool reorient_normals)
{
    assert_true(nlights == lights.size());
    std::stringstream sstr;
    sstr << "#version 330 core" << std::endl;
    sstr << "in vec3 color;" << std::endl;
    if (has_texture) {
        sstr << "in vec2 tex_coord;" << std::endl;
    }
    sstr << "out vec4 frag_color;" << std::endl;
    sstr << "uniform sampler2D texture1;" << std::endl;
    if (has_lightmap_color || has_lightmap_depth) {
        sstr << "in vec4 FragPosLightSpace[" << lights.size() << "];" << std::endl;
    }
    assert_true(!(has_lightmap_color && has_lightmap_depth));
    if (has_lightmap_color) {
        sstr << "uniform sampler2D texture_light_color[" << lights.size() << "];" << std::endl;
    }
    if (has_lightmap_depth) {
        sstr << "uniform sampler2D texture_light_depth[" << lights.size() << "];" << std::endl;
    }
    if (has_dirtmap) {
        sstr << "in vec2 tex_coord_dirtmap;" << std::endl;
        sstr << "uniform sampler2D texture_dirtmap;" << std::endl;
        sstr << "uniform sampler2D texture_dirt;" << std::endl;
    }
    if (diffusivity.is_nonzero() || specularity.is_nonzero()) {
        sstr << "in vec3 Normal;" << std::endl;

        // sstr << "uniform vec3 lightPos;" << std::endl;
        sstr << "uniform vec3 lightDir[" << lights.size() << "];" << std::endl;
        sstr << "uniform vec3 lightColor[" << lights.size() << "];" << std::endl;
    }
    if (reorient_normals || specularity.is_nonzero()) {
        sstr << "in vec3 FragPos;" << std::endl;
        sstr << "uniform vec3 viewPos;" << std::endl;
    }
    sstr << "void main()" << std::endl;
    sstr << "{" << std::endl;
    if (alpha_threshold < 1) {
        if (!has_texture) {
            throw std::runtime_error("Alpha threshold requires texture");
        }
        sstr << "    vec4 tex_color = texture(texture1, tex_coord);" << std::endl;
        sstr << "    if (tex_color.a < " << alpha_threshold << ")" << std::endl;
        sstr << "        discard;" << std::endl;
    }
    FixedArray<float, 3> am{ambience};
    FixedArray<float, 3> di{diffusivity};
    FixedArray<float, 3> sp{specularity};
    if (!lights.empty()) {
        am *= lights.front().second->ambience;
        di *= lights.front().second->diffusivity;
        sp *= lights.front().second->specularity;
    }
    sstr << "    vec4 fragBrightness = vec4(" << am(0) << ", " << am(1) << ", " << am(2) << ", 1);" << std::endl;
    if (diffusivity.is_nonzero() || specularity.is_nonzero()) {
        sstr << "    vec3 norm = normalize(Normal);" << std::endl;
        // sstr << "    vec3 lightDir = normalize(lightPos - FragPos);" << std::endl;
    }
    if (reorient_normals) {
        sstr << "    norm *= sign(dot(norm, viewPos - FragPos));" << std::endl;
    }
    if (has_lightmap_color) {
        sstr << "    vec4 color_fac = vec4(1, 1, 1, 1);" << std::endl;
    }
    if (has_lightmap_color || has_lightmap_depth) {
        sstr << "    for (int i = 0; i < " << lights.size() << "; ++i) {" << std::endl;
        sstr << "        vec3 proj_coords11 = FragPosLightSpace[i].xyz / FragPosLightSpace[i].w;" << std::endl;
        sstr << "        vec3 proj_coords01 = proj_coords11 * 0.5 + 0.5;" << std::endl;
    }
    assert_true(!(has_lightmap_color && has_lightmap_depth));
    if (has_lightmap_color) {
        sstr << "        color_fac *= texture(texture_light_color[i], proj_coords01.xy);" << std::endl;
        sstr << "    }" << std::endl;
    }
    if (has_lightmap_depth) {
        sstr << "        if (proj_coords01.z - 0.00002 < texture(texture_light_depth[i], proj_coords01.xy).r) {" << std::endl;
    }
    if (!has_lightmap_depth && !lights.empty()) {
        sstr << "    {" << std::endl;
        sstr << "        int i = 0;" << std::endl;
    }
    if (diffusivity.is_nonzero()) {
        sstr << "            vec3 fragDiffusivity = vec3(" << di(0) << ", " << di(1) << ", " << di(2) << ");" << std::endl;
        sstr << "            float diff = max(dot(norm, lightDir[i]), 0.0);" << std::endl;
        sstr << "            vec3 diffuse = fragDiffusivity * diff * lightColor[i];" << std::endl;
        sstr << "            fragBrightness += vec4(diffuse, 0);" << std::endl;
    }
    if (specularity.is_nonzero()) {
        sstr << "            vec3 fragSpecularity = vec3(" << sp(0) << ", " << sp(1) << ", " << sp(2) << ");" << std::endl;
        sstr << "            vec3 viewDir = normalize(viewPos - FragPos);" << std::endl;
        sstr << "            vec3 reflectDir = reflect(-lightDir[i], norm);  " << std::endl;
        sstr << "            float spec = pow(max(dot(viewDir, reflectDir), 0.0), 4);" << std::endl;
        sstr << "            vec3 specular = fragSpecularity * spec * lightColor[i];" << std::endl;
        sstr << "            fragBrightness += vec4(specular, 0);" << std::endl;
    }
    if (has_lightmap_depth) {
        sstr << "        }" << std::endl;
    }
    if (has_lightmap_color || has_lightmap_depth || !lights.empty()) {
        sstr << "    }" << std::endl;
    }
    if (has_lightmap_color) {
        sstr << "    fragBrightness *= color_fac;" << std::endl;
    }
    if (!has_texture && has_dirtmap) {
        std::runtime_error("Combination of (!has_texture && has_dirtmap) is not supported");
    }
    if (has_dirtmap) {
        sstr << "    vec4 dirtiness = texture(texture_dirtmap, tex_coord_dirtmap);" << std::endl;
        sstr << "    dirtiness.r = clamp(0.5 + 4 * (dirtiness.r - 0.5), 0, 1);" << std::endl;
        sstr << "    dirtiness.g = clamp(0.5 + 4 * (dirtiness.g - 0.5), 0, 1);" << std::endl;
        sstr << "    dirtiness.b = clamp(0.5 + 4 * (dirtiness.b - 0.5), 0, 1);" << std::endl;
        sstr << "    frag_color = texture(texture1, tex_coord) * (1 - dirtiness)" << std::endl;
        sstr << "               + texture(texture_dirt, tex_coord) * dirtiness;" << std::endl;
        sstr << "    frag_color *= vec4(color, 1.0);" << std::endl;
    } else if (has_texture) {
        sstr << "    frag_color = texture(texture1, tex_coord) * vec4(color, 1.0);" << std::endl;
    } else {
        sstr << "    frag_color = vec4(color, 1.0);" << std::endl;
    }
    sstr << "    frag_color = frag_color * fragBrightness;" << std::endl;
    if (occlusion_type == OcclusionType::OCCLUDED) {
        sstr << "    frag_color.r = 1;" << std::endl;
        sstr << "    frag_color.g = 1;" << std::endl;
        sstr << "    frag_color.b = 1;" << std::endl;
    }
    if (occlusion_type == OcclusionType::OCCLUDER) {
        sstr << "    frag_color.r = 0.5;" << std::endl;
        sstr << "    frag_color.g = 0.5;" << std::endl;
        sstr << "    frag_color.b = 0.5;" << std::endl;
    }
    sstr << "}" << std::endl;
    return sstr.str();
}};

RenderableColoredVertexArray::RenderableColoredVertexArray(
    const std::list<std::shared_ptr<ColoredVertexArray>>& triangles,
    RenderingResources* rendering_resources)
: triangles_res_{triangles},
  rendering_resources_{rendering_resources},
  render_textures_{rendering_resources_ != nullptr}
{}

RenderableColoredVertexArray::RenderableColoredVertexArray(
    const std::shared_ptr<ColoredVertexArray>& triangles,
    RenderingResources* rendering_resources)
: RenderableColoredVertexArray(std::list<std::shared_ptr<ColoredVertexArray>>{triangles}, rendering_resources)
{}

void RenderableColoredVertexArray::instantiate_renderable(const std::string& name, SceneNode& scene_node, const SceneNodeResourceFilter& resource_filter)
{
    scene_node.add_renderable(name, std::make_shared<RenderableColoredVertexArrayInstance>(
        shared_from_this(),
        resource_filter));
}

std::list<std::shared_ptr<ColoredVertexArray>> RenderableColoredVertexArray::get_triangle_meshes() {
    return triangles_res_;
}

void RenderableColoredVertexArray::generate_triangle_rays(size_t npoints, const FixedArray<float, 3>& lengths, bool delete_triangles) {
    for(auto& t : triangles_res_) {
        auto r = Mlib::generate_triangle_rays(t->triangles, npoints, lengths);
        t->lines.reserve(t->lines.size() + r.size());
        for(const auto& l : r) {
            t->lines.push_back({
                ColoredVertex{
                    position: l(0),
                    color: {1, 1, 1},
                    uv: {0, 0}
                },
                ColoredVertex{
                    position: l(1),
                    color: {1, 1, 1},
                    uv: {0, 1}
                }
            });
        }
        if (delete_triangles) {
            t->triangles.clear();
        }
    }
}

void RenderableColoredVertexArray::generate_ray(const FixedArray<float, 3>& from, const FixedArray<float, 3>& to) {
    if (triangles_res_.size() != 1) {
        throw std::runtime_error("generate_ray requires exactly one triangle mesh");
    }
    triangles_res_.front()->lines.push_back({
        ColoredVertex{
            position: from,
            color: {1, 1, 1},
            uv: {0, 0}
        },
        ColoredVertex{
            position: to,
            color: {1, 1, 1},
            uv: {0, 1}
        }
    });
}

const ColoredRenderProgram& RenderableColoredVertexArray::get_render_program(
    const RenderProgramIdentifier& id,
    const std::list<std::pair<FixedArray<float, 4, 4>, Light*>>& filtered_lights) const
{
    if (id.aggregate_mode != AggregateMode::OFF) {
        throw std::runtime_error("get_render_program called on aggregated material");
    }
    if (auto it = render_programs_.find(id); it != render_programs_.end()) {
        return *it->second;
    }
    std::lock_guard guard{mutex_};
    if (auto it = render_programs_.find(id); it != render_programs_.end()) {
        return *it->second;
    }
    auto rp = std::make_unique<ColoredRenderProgram>();
    OcclusionType occlusion_type;
    if (id.calculate_lightmap) {
        if (id.occluder_type == OccluderType::WHITE) {
            occlusion_type = OcclusionType::OCCLUDED;
        } else if (id.occluder_type == OccluderType::BLACK) {
            occlusion_type = OcclusionType::OCCLUDER;
        } else {
            throw std::runtime_error("get_render_program: calculate_lightmap requires occlusion");
        }
    } else {
        occlusion_type = OcclusionType::OFF;
    }
    rp->generate(
        vertex_shader_text_gen(
            filtered_lights,
            filtered_lights.size(),
            id.has_lightmap_color,
            id.has_lightmap_depth,
            id.has_dirtmap,
            id.diffusivity.is_nonzero(),
            id.specularity.is_nonzero(),
            id.reorient_normals),
        fragment_shader_text_textured_rgb_gen(
            filtered_lights,
            filtered_lights.size(),
            id.has_texture,
            id.has_lightmap_color,
            id.has_lightmap_depth,
            id.has_dirtmap,
            OrderableFixedArray{id.ambience},
            OrderableFixedArray{id.diffusivity},
            OrderableFixedArray{id.specularity},
            id.blend_mode == BlendMode::BINARY
                ? (id.calculate_lightmap ? 0.1 : 0.9)
                : 1,
            occlusion_type,
            id.reorient_normals));

    rp->mvp_location = checked_glGetUniformLocation(rp->program, "MVP");
    if (id.has_texture) {
        rp->texture1_location = checked_glGetUniformLocation(rp->program, "texture1");
    } else {
        rp->texture1_location = 0;
    }
    if (id.has_lightmap_color || id.has_lightmap_depth) {
        for(size_t i = 0; i < filtered_lights.size(); ++i) {
            rp->mvp_light_locations[i] = checked_glGetUniformLocation(rp->program, ("MVP_light[" + std::to_string(i) + "]").c_str());
        }
    } else {
        // Do nothing
        // rp->mvp_light_location = 0;
    }
    assert(!(id.has_lightmap_color && id.has_lightmap_depth));
    if (id.has_lightmap_color) {
        for(size_t i = 0; i < filtered_lights.size(); ++i) {
            rp->texture_lightmap_color_locations[i] = checked_glGetUniformLocation(rp->program, ("texture_light_color[" + std::to_string(i) + "]").c_str());
        }
    } else {
        // Do nothing
        // rp->texture_lightmap_color_location = 0;
    }
    if (id.has_lightmap_depth) {
        for(size_t i = 0; i < filtered_lights.size(); ++i) {
            rp->texture_lightmap_depth_locations[i] = checked_glGetUniformLocation(rp->program, ("texture_light_depth[" + std::to_string(i) + "]").c_str());
        }
    } else {
        // Do nothing
        // rp->texture_lightmap_depth_location = 0;
    }
    if (id.has_dirtmap) {
        rp->mvp_dirtmap_location = checked_glGetUniformLocation(rp->program, "MVP_dirtmap");
        rp->texture_dirtmap_location = checked_glGetUniformLocation(rp->program, "texture_dirtmap");
        rp->texture_dirt_location = checked_glGetUniformLocation(rp->program, "texture_dirt");
    } else {
        rp->mvp_dirtmap_location = 0;
        rp->texture_dirtmap_location = 0;
        rp->texture_dirt_location = 0;
    }
    if (id.diffusivity.is_nonzero() || id.specularity.is_nonzero()) {
        rp->m_location = checked_glGetUniformLocation(rp->program, "M");
        // rp->light_position_location = checked_glGetUniformLocation(rp->program, "lightPos");
        for(size_t i = 0; i < filtered_lights.size(); ++i) {
            rp->light_dir_locations[i] = checked_glGetUniformLocation(rp->program, ("lightDir[" + std::to_string(i) + "]").c_str());
            rp->light_colors[i] = checked_glGetUniformLocation(rp->program, ("lightColor[" + std::to_string(i) + "]").c_str());
        }
    } else {
        rp->m_location = 0;
        // rp->light_position_location = 0;
        // Do nothing
        // rp->light_dir_location = 0;
        // rp->light_color = 0;
    }
    if (id.specularity.is_nonzero()) {
        rp->view_pos = checked_glGetUniformLocation(rp->program, "viewPos");
    } else {
        rp->view_pos = 0;
    }

    auto& result = *rp;
    render_programs_.insert(std::make_pair(id, std::move(rp)));
    return result;
}

const VertexArray& RenderableColoredVertexArray::get_vertex_array(const ColoredVertexArray* cva) const {
    if (cva->material.aggregate_mode != AggregateMode::OFF) {
        throw std::runtime_error("get_vertex_array called on aggregated object");
    }
    if (auto it = vertex_arrays_.find(cva); it != vertex_arrays_.end()) {
        return *it->second;
    }
    std::lock_guard guard{mutex_};
    auto va = std::make_unique<VertexArray>();
    // https://stackoverflow.com/a/13405205/2292832
    CHK(glGenVertexArrays(1, &va->vertex_array));
    CHK(glBindVertexArray(va->vertex_array));

    CHK(glGenBuffers(1, &va->vertex_buffer));
    CHK(glBindBuffer(GL_ARRAY_BUFFER, va->vertex_buffer));
    CHK(glBufferData(GL_ARRAY_BUFFER, sizeof((cva->triangles)[0]) * cva->triangles.size(), cva->triangles.begin()->flat_begin(), GL_STATIC_DRAW));

    CHK(glEnableVertexAttribArray(0));
    CHK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                            sizeof((cva->triangles)[0][0]), (void*) 0));
    CHK(glEnableVertexAttribArray(1));
    CHK(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                            sizeof((cva->triangles)[0][0]), (void*) (sizeof(float) * 3)));
    CHK(glEnableVertexAttribArray(2));
    CHK(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,
                            sizeof((cva->triangles)[0][0]), (void*) (sizeof(float) * 6)));
    CHK(glEnableVertexAttribArray(3));
    CHK(glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE,
                            sizeof((cva->triangles)[0][0]), (void*) (sizeof(float) * 8)));

    CHK(glBindVertexArray(0));
    auto& result = *va;
    vertex_arrays_.insert(std::make_pair(cva, std::move(va)));
    return result;
}
