#include "Skybox_Logic.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Rendered_Scene_Descriptor.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>

using namespace Mlib;

static const char* vertex_shader_text =
    "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "\n"
    "out vec3 TexCoords;\n"
    "\n"
    "uniform mat4 vp;\n"
    "\n"
    "void main()\n"
    "{\n"
    "    TexCoords = aPos;\n"
    "    vec4 pos = vp * vec4(aPos, 1.0);\n"
    "    gl_Position = pos.xyzz;\n"
    "}";

static const char* fragment_shader_text =
    "#version 330 core\n"
    "out vec4 FragColor;\n"
    "\n"
    "in vec3 TexCoords;\n"
    "\n"
    "uniform samplerCube skybox;\n"
    "\n"
    "void main()\n"
    "{\n"
    "    FragColor = texture(skybox, TexCoords);\n"
    "}";

float skybox_vertices[] = {
    // positions          
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,
    1.0f,  1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,
    1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
    1.0f,  1.0f, -1.0f,
    1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
    1.0f, -1.0f,  1.0f
};

SkyboxLogic::SkyboxLogic(RenderLogic& child_logic)
: child_logic_{child_logic},
  rendering_context_{RenderingContextStack::rendering_context()},
  loaded_{false}
{}

SkyboxLogic::~SkyboxLogic() {}

void SkyboxLogic::render(
    int width,
    int height,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    if (!loaded_) {
        loaded_ = true;
        if (!filenames_.empty()) {
            rp_.generate(vertex_shader_text, fragment_shader_text);
            rp_.skybox_location = checked_glGetUniformLocation(rp_.program, "skybox");
            rp_.vp_location = checked_glGetUniformLocation(rp_.program, "vp");

            CHK(glGenVertexArrays(1, &va_.vertex_array));
            CHK(glGenBuffers(1, &va_.vertex_buffer));
            CHK(glBindVertexArray(va_.vertex_array));
            CHK(glBindBuffer(GL_ARRAY_BUFFER, va_.vertex_buffer));
            CHK(glBufferData(GL_ARRAY_BUFFER, sizeof(skybox_vertices), &skybox_vertices, GL_STATIC_DRAW));
            CHK(glEnableVertexAttribArray(0));
            CHK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr));
            CHK(glBindVertexArray(0));
        }
    }
    LOG_FUNCTION("SkyboxLogic::render");
    {
        RenderingContextGuard rrg{rendering_context_};
        child_logic_.render(width, height, render_config, scene_graph_config, render_results, frame_id);
    }
    if (!filenames_.empty() && (frame_id.external_render_pass.pass != ExternalRenderPass::LIGHTMAP_TO_TEXTURE)) {
        CHK(glEnable(GL_DEPTH_TEST));
        CHK(glDepthFunc(GL_LEQUAL));  // change depth function so depth test passes when values are equal to depth buffer's content
        CHK(glUseProgram(rp_.program));

        FixedArray<float, 4, 4> vp = child_logic_.vp();
        vp(0, 3) = 0;
        vp(1, 3) = 0;
        vp(2, 3) = 0;
        vp(3, 3) = 1;
        CHK(glUniformMatrix4fv(rp_.vp_location, 1, GL_TRUE, (const GLfloat*) vp.flat_begin()));

        CHK(glUniform1i(rp_.skybox_location, 0));
        CHK(glActiveTexture(GL_TEXTURE0));
        CHK(glBindTexture(GL_TEXTURE_CUBE_MAP, rendering_context_.rendering_resources->get_cubemap(alias_, filenames_)));

        CHK(glBindVertexArray(va_.vertex_array));
        CHK(glDrawArrays(GL_TRIANGLES, 0, 36));

        // Reset to defaults
        CHK(glBindVertexArray(0));
        CHK(glDepthFunc(GL_LESS));
        CHK(glDisable(GL_DEPTH_TEST));
    }
}

float SkyboxLogic::near_plane() const {
    return child_logic_.near_plane();
}

float SkyboxLogic::far_plane() const {
    return child_logic_.far_plane();
}

const FixedArray<float, 4, 4>& SkyboxLogic::vp() const {
    return child_logic_.vp();
}

const TransformationMatrix<float, 3>& SkyboxLogic::iv() const {
    return child_logic_.iv();
}

bool SkyboxLogic::requires_postprocessing() const {
    return child_logic_.requires_postprocessing();
}

void SkyboxLogic::set_filenames(const std::vector<std::string>& filenames, const std::string& alias) {
    if (!filenames_.empty()) {
        throw std::runtime_error("SkyboxLogic::set_filenames called multiple times");
    }
    filenames_ = filenames;
    alias_ = alias;
}
