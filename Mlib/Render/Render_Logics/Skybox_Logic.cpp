#include "Skybox_Logic.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Cameras/Camera.hpp>
#include <Mlib/Geometry/Material/Colormap_With_Modifiers.hpp>
#include <Mlib/Geometry/Texture/ITexture_Handle.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Render_Setup.hpp>
#include <Mlib/Render/Rendered_Scene_Descriptor.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Resource_Managers/Rendering_Resources.hpp>
#include <Mlib/Render/Shader_Version_3_0.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

static const char* vertex_shader_text =
    SHADER_VER
    "layout (location = 0) in vec3 aPos;\n"
    "\n"
    "out vec3 TexCoords;\n"
    "\n"
    "uniform mat4 vp;\n"
    "\n"
    "void main()\n"
    "{\n"
    // "    TexCoords = aPos;\n"
    // Modification proposed in https://learnopengl.com/Advanced-OpenGL/Cubemaps#comment-5197766106
    // This works in combination with not flipping the y-coordinate when loading the texture.
    "    TexCoords = vec3(aPos.xy, -aPos.z);\n"
    "    vec4 pos = vp * vec4(aPos, 1.0);\n"
    "    gl_Position = pos.xyzz;\n"
    "}";

static const char* fragment_shader_text =
    SHADER_VER     FRAGMENT_PRECISION
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
    : child_logic_{ child_logic }
    , rendering_resources_{ RenderingContextStack::primary_rendering_resources() }
{
    va_.add_array_buffer(vertices_);
}

SkyboxLogic::~SkyboxLogic() {
    on_destroy.clear();
}

std::optional<RenderSetup> SkyboxLogic::try_render_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderedSceneDescriptor& frame_id) const
{
    return child_logic_.render_setup(lx, ly, frame_id);
}

void SkyboxLogic::render_with_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id,
    const RenderSetup& setup)
{
    LOG_FUNCTION("SkyboxLogic::render");
    if (texture_ == nullptr) {
        return;
    }
    // TimeGuard time_guard{"SkyboxLogic::render", "SkyboxLogic::render"};
    if (!rp_.allocated()) {
        rp_.allocate(vertex_shader_text, fragment_shader_text);
        rp_.skybox_location = rp_.get_uniform_location("skybox");
        rp_.vp_location = rp_.get_uniform_location("vp");

        va_.initialize();
        vertices_.set(skybox_vertices, TaskLocation::FOREGROUND);
        CHK(glEnableVertexAttribArray(0));
        CHK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr));
        CHK(glBindVertexArray(0));
    }
    if (frame_id.external_render_pass.pass == ExternalRenderPassType::STANDARD) {
        CHK(glEnable(GL_DEPTH_TEST));
        CHK(glDepthFunc(GL_LEQUAL));  // change depth function so depth test passes when values are equal to depth buffer's content
        rp_.use();

        FixedArray<float, 4, 4> vp = setup.vp.casted<float>();
        vp(0, 3) = 0;
        vp(1, 3) = 0;
        vp(2, 3) = 0;
        vp(3, 3) = 1;
        CHK(glUniformMatrix4fv(rp_.vp_location, 1, GL_TRUE, vp.flat_begin()));

        CHK(glUniform1i(rp_.skybox_location, 0));
        CHK(glActiveTexture(GL_TEXTURE0));
        CHK(glBindTexture(GL_TEXTURE_CUBE_MAP, texture_->handle<GLuint>()));

        va_.bind();
        CHK(glDrawArrays(GL_TRIANGLES, 0, 36));

        // Reset to defaults
        CHK(glBindVertexArray(0));
        CHK(glDepthFunc(GL_LESS));
        CHK(glDisable(GL_DEPTH_TEST));
    }
}

void SkyboxLogic::clear_alias() {
    texture_ = nullptr;
}

void SkyboxLogic::set_alias(VariableAndHash<std::string> alias) {
    if (texture_ != nullptr) {
        THROW_OR_ABORT("SkyboxLogic::set_alias called multiple times");
    }
    texture_ = rendering_resources_.get_texture_lazy(ColormapWithModifiers{
        .filename = alias,
        .color_mode = ColorMode::RGB,
        .mipmap_mode = MipmapMode::WITH_MIPMAPS}.compute_hash());
}

void SkyboxLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "SkyboxLogic\n";
    child_logic_.print(ostr, depth + 1);
}
