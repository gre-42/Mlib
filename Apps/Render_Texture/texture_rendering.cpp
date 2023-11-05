#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Material/Color_Mode.hpp>
#include <Mlib/Geometry/Material/Texture_Descriptor.hpp>
#include <Mlib/Geometry/Mesh/Load/Load_Kn5.hpp>
#include <Mlib/Geometry/Texture/Uv_Tile.hpp>
#include <Mlib/Iterator/Enumerate.hpp>
#include <Mlib/Memory/Destruction_Guard.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Context_Query.hpp>
#include <Mlib/Render/Gl_Context_Guard.hpp>
#include <Mlib/Render/Particle_Resources.hpp>
#include <Mlib/Render/Render_Logics/Fill_With_Texture_Logic.hpp>
#include <Mlib/Render/Render_Logics/Resource_Update_Cycle.hpp>
#include <Mlib/Render/Render_Texture_Atlas.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Render/Text/Renderable_Text.hpp>
#include <Mlib/Render/Window.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Threads/Realtime_Threads.hpp>
#include <iostream>

using namespace Mlib;

int main(int argc, char** argv)
{
    ArgParser parser(
        "Usage: render_texture {<texture>, --kn5 container.kn5 <texture_regex> [--size <n>] --mip_level_count <n> --mip_level <n> --atlas_layer <n> [--rerender_atlas] [--texname]}",
        {"--rerender_atlas"},
        {"--kn5", "--size", "--mip_level_count", "--mip_level", "--atlas_layer", "--texname"});

    try {
        auto parsed = parser.parsed(argc, argv);

        parsed.assert_num_unnamed(1);

        reserve_realtime_threads(0);

        // glfw: initialize and configure
        // ------------------------------
        GLFW_CHK(glfwInit());
        DestructionGuard dg{[](){glfwTerminate();}};
        GLFW_CHK(glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4));
        GLFW_CHK(glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1));
        GLFW_CHK(glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE));

    #ifdef __APPLE__
        GLFW_CHK(glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE));
    #endif

        // glfw window creation
        // --------------------
        Window window{
            800,
            600,
            "LearnOpenGL",
            nullptr,    // monitor
            nullptr,    // share
            true,       // use_double_buffering
            1,          // swap_interval
            0};         // fullscreen_refresh_rate
        GlContextGuard gcg{ window };
        CHK(int version = gladLoadGL(glfwGetProcAddress));
        if (version == 0) {
            throw std::runtime_error("gladLoadGL failed");
        }

        // Resources
        // ---------
        SceneNodeResources scene_node_resources;
        ParticleResources particle_resources;
        RenderingResources rendering_resources{
            "primary_rendering_resources",
            16 };
        RenderingContext primary_rendering_context{
            .scene_node_resources = scene_node_resources,
            .particle_resources = particle_resources,
            .rendering_resources = rendering_resources,
            .z_order = 0 };
        RenderingContextGuard rcg{ primary_rendering_context };

        // OpenGL state
        // ------------
        AutoTextureAtlasDescriptor atlas;
        std::optional<FillWithTextureLogic> ftl;
        if (parsed.has_named_value("--kn5")) {
            auto kn5 = load_kn5(parsed.named_value("--kn5"));
            std::list<std::string> names;
            static const DECLARE_REGEX(re, parsed.unnamed_value(0));
            for (auto& [name, data] : kn5.textures) {
                if (!Mlib::re::regex_search(name, re)) {
                    continue;
                }
                linfo() << "Matched: " << name;
                names.push_back(name);
                rendering_resources.insert_texture(
                    name,
                    std::move(data.data),
                    TextureAlreadyExistsBehavior::RAISE);
            }
            if (names.empty()) {
                THROW_OR_ABORT("Could not find a single texture matching \"" + parsed.unnamed_value(0) + '"');
            }
            rendering_resources.generate_auto_texture_atlas(
                "__texture__",
                std::vector(names.begin(), names.end()),
                safe_stoi(parsed.named_value("--mip_level_count")),
                safe_stoi(parsed.named_value("--size", "4096")),
                &atlas);
            for (const auto& [i, t] : enumerate(atlas.tiles)) {
                linfo() << "Layer: " << i;
                for (const auto& d : t) {
                    linfo() << "  Filename: " << d.filename << ' ' << d.width << 'x' << d.height;
                }
            }
            if (parsed.has_named_value("--texname")) {
                ftl.emplace(
                    rendering_resources,
                    ColormapWithModifiers{
                        .filename = parsed.named_value("--texname"),
                        .color_mode = ColorMode::RGBA,
                        .mipmap_mode = MipmapMode::WITH_MIPMAPS
                    },
                    ResourceUpdateCycle::ONCE);
            } else if (!parsed.has_named("--rerender_atlas")) {
                auto layer = safe_stoz(parsed.named_value("--atlas_layer"));
                if (layer >= atlas.tiles.size()) {
                    THROW_OR_ABORT("Layer index out of bounds");
                }
                ftl.emplace(
                    rendering_resources,
                    ColormapWithModifiers{
                        .filename = "__texture__",
                        .color_mode = ColorMode::RGBA,
                        .mipmap_mode = MipmapMode::WITH_MIPMAPS},
                    ResourceUpdateCycle::ONCE,
                    CullFaceMode::CULL,
                    AlphaChannelRole::BLEND,
                    standard_quad_vertices,
                    layer);
                rendering_resources.preload({
                    .color = {
                        .filename = "__texture__",
                        .color_mode = ColorMode::RGBA,
                        .mipmap_mode = MipmapMode::WITH_MIPMAPS
                    }});
            }
        } else {
            rendering_resources.add_texture_descriptor(
                "__texture__",
                TextureDescriptor{
                    .color = {
                        .filename = parsed.unnamed_value(0),
                        .color_mode = ColorMode::RGBA,
                        .mipmap_mode = MipmapMode::WITH_MIPMAPS}});
            ftl.emplace(
                rendering_resources,
                ColormapWithModifiers{
                    .filename = "__texture__",
                    .color_mode = ColorMode::RGBA,
                    .mipmap_mode = MipmapMode::WITH_MIPMAPS},
                ResourceUpdateCycle::ONCE);
        }

        // render loop
        // -----------
        while (!glfwWindowShouldClose(&window.glfw_window()))
        {
            int width, height;

            GLFW_CHK(glfwGetFramebufferSize(&window.glfw_window(), &width, &height));
            CHK(glViewport(0, 0, width, height));

            // render
            // ------
            glClearColor(1.f, 0.f, 1.f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            if (ftl.has_value()) {
                // CHK(glViewport(0, 0, 1024, 1024));
                ftl.value().render();
            } else if (!atlas.tiles.empty()) {
                auto layer = safe_stoz(parsed.named_value("--atlas_layer"));
                if (layer >= atlas.tiles.size()) {
                    THROW_OR_ABORT("Layer index out of bounds");
                }
                float scale = std::pow(2.f, -safe_stof(parsed.named_value("--mip_level")));
                if (std::isnan(scale)) {
                    THROW_OR_ABORT("Scale is NAN");
                }
                render_texture_atlas(
                    rendering_resources,
                    atlas.tiles.at(layer),
                    scale,
                    scale);
            }

            // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
            // -------------------------------------------------------------------------------
            window.draw();
            GLFW_CHK(glfwPollEvents());
        }
    } catch (const std::exception& e) {
        lerr() << e.what();
        return 1;
    }
}
