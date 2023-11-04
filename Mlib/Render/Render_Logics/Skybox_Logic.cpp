#include "Skybox_Logic.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Material/Colormap_With_Modifiers.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Deallocate/Render_Deallocator.hpp>
#include <Mlib/Render/Rendered_Scene_Descriptor.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Render/Shader_Version.hpp>
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
: child_logic_{child_logic},
  rendering_resources_{RenderingContextStack::primary_rendering_resources()},
  loaded_{false},
  deallocation_token_{render_deallocator.insert([this](){deallocate();})}
{}

SkyboxLogic::~SkyboxLogic() = default;

void SkyboxLogic::deallocate() {
    loaded_ = false;
}

void SkyboxLogic::render(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    LOG_FUNCTION("SkyboxLogic::render");
    // TimeGuard time_guard{"SkyboxLogic::render", "SkyboxLogic::render"};
    if (!loaded_) {
        loaded_ = true;
        if (!alias_.empty()) {
            rp_.allocate(vertex_shader_text, fragment_shader_text);
            rp_.skybox_location = checked_glGetUniformLocation(rp_.program, "skybox");
            rp_.vp_location = checked_glGetUniformLocation(rp_.program, "vp");

            va_.initialize();
            va_.vertex_buffer.set(skybox_vertices);
            CHK(glEnableVertexAttribArray(0));
            CHK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr));
            CHK(glBindVertexArray(0));
        }
    }
    child_logic_.render(
        lx,
        ly,
        render_config,
        scene_graph_config,
        render_results,
        frame_id);
    if (!alias_.empty() && (frame_id.external_render_pass.pass == ExternalRenderPassType::STANDARD)) {
        CHK(glEnable(GL_DEPTH_TEST));
        CHK(glDepthFunc(GL_LEQUAL));  // change depth function so depth test passes when values are equal to depth buffer's content
        CHK(glUseProgram(rp_.program));

        FixedArray<float, 4, 4> vp = child_logic_.vp().casted<float>();
        vp(0u, 3u) = 0;
        vp(1u, 3u) = 0;
        vp(2u, 3u) = 0;
        vp(3u, 3u) = 1;
        CHK(glUniformMatrix4fv(rp_.vp_location, 1, GL_TRUE, vp.flat_begin()));

        CHK(glUniform1i(rp_.skybox_location, 0));
        CHK(glActiveTexture(GL_TEXTURE0));
        CHK(glBindTexture(GL_TEXTURE_CUBE_MAP, rendering_resources_.get_texture({
            .filename = alias_,
            .color_mode = ColorMode::RGB,
            .mipmap_mode = MipmapMode::WITH_MIPMAPS})));

        CHK(glBindVertexArray(va_.vertex_array()));
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

const FixedArray<double, 4, 4>& SkyboxLogic::vp() const {
    return child_logic_.vp();
}

const TransformationMatrix<float, double, 3>& SkyboxLogic::iv() const {
    return child_logic_.iv();
}

DanglingRef<const SceneNode> SkyboxLogic::camera_node() const {
    return child_logic_.camera_node();
}

bool SkyboxLogic::requires_postprocessing() const {
    return child_logic_.requires_postprocessing();
}

void SkyboxLogic::set_alias(const std::string& alias) {
    if (!alias_.empty()) {
        THROW_OR_ABORT("SkyboxLogic::set_alias called multiple times");
    }
    alias_ = alias;
}

void SkyboxLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "SkyboxLogic\n";
    child_logic_.print(ostr, depth + 1);
}
