#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Material/Color_Mode.hpp>
#include <Mlib/Geometry/Material/Texture_Descriptor.hpp>
#include <Mlib/Geometry/Mesh/Load/Load_Dff.hpp>
#include <Mlib/Geometry/Mesh/Load/Load_Kn5.hpp>
#include <Mlib/Geometry/Mesh/Load/Raster_Config.hpp>
#include <Mlib/Geometry/Texture/Uv_Tile.hpp>
#include <Mlib/Images/Flip_Mode.hpp>
#include <Mlib/Io/Arg_Parser.hpp>
#include <Mlib/Iterator/Enumerate.hpp>
#include <Mlib/Memory/Destruction_Guard.hpp>
#include <Mlib/OpenGL/CHK.hpp>
#include <Mlib/OpenGL/Context_Query.hpp>
#include <Mlib/OpenGL/Gl_Context_Guard.hpp>
#include <Mlib/OpenGL/OpenGL_Object_Factory.hpp>
#include <Mlib/OpenGL/Raster/Raster_Factory.hpp>
#include <Mlib/OpenGL/Render_Logics/Clear_Mode.hpp>
#include <Mlib/OpenGL/Render_Logics/Fill_With_Texture_Logic.hpp>
#include <Mlib/OpenGL/Render_Logics/Resource_Update_Cycle.hpp>
#include <Mlib/OpenGL/Render_Texture_Atlas.hpp>
#include <Mlib/OpenGL/Renderables/OpenGL_Vertex_Array_Renderer.hpp>
#include <Mlib/OpenGL/Rendering_Context.hpp>
#include <Mlib/OpenGL/Resource_Managers/Particle_Resources.hpp>
#include <Mlib/OpenGL/Resource_Managers/Rendering_Resources.hpp>
#include <Mlib/OpenGL/Resource_Managers/Trail_Resources.hpp>
#include <Mlib/OpenGL/Text/Renderable_Text.hpp>
#include <Mlib/OpenGL/Window.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Scene_Graph/Render/Caching_Gpu_Object_Factory.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Strings/U32_Regex.hpp>
#include <Mlib/Threads/Realtime_Threads.hpp>

using namespace Mlib;

static const auto tmp_texture = VariableAndHash<std::string>{ "__texture__" };

int main(int argc, char** argv)
{
    ArgParser parser(
        "Usage: render_texture {<texture>, --kn5 container.kn5 --filter <regex> [--size <n>] --mip_level_count <n> --mip_level <n> [--aniso <n>] --atlas_layer <n> [--rerender_atlas] [--texname]}",
        {"--rerender_atlas"},
        {"--kn5", "--txd", "--filter", "--size", "--mip_level_count", "--mip_level", "--aniso", "--atlas_layer", "--texname"});

    try {
        auto args = parser.parsed(argc, argv);

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
        if (int version = gladLoadGL(glfwGetProcAddress); version == 0) {
            throw std::runtime_error("gladLoadGL failed");
        }

        // Resources
        // ---------
        OpenGLObjectFactory gpu_object_factory;
        CachingGpuObjectFactory caching_gpu_object_factory{gpu_object_factory};
        SceneNodeResources scene_node_resources{caching_gpu_object_factory};
        ParticleResources particle_resources;
        TrailResources trail_resources;
        RenderingResources rendering_resources{
            "primary_rendering_resources",
            16 };
        OpenGLVertexArrayRenderer gpu_vertex_array_renderer{rendering_resources, rendering_resources};
        RenderingContext primary_rendering_context{
            .scene_node_resources = scene_node_resources,
            .particle_resources = particle_resources,
            .trail_resources = trail_resources,
            .rendering_resources = rendering_resources,
            .gpu_object_factory = caching_gpu_object_factory,
            .gpu_vertex_array_renderer = gpu_vertex_array_renderer,
            .z_order = 0 };
        RenderingContextGuard rcg{ primary_rendering_context };

        // OpenGL state
        // ------------
        AutoTextureAtlasDescriptor atlas;
        std::optional<FillWithTextureLogic> ftl;
        auto unnamed = args.unnamed_values();
        if (unnamed.empty()) {
            static const auto re = Mlib::make_u32regex(args.named_value("--filter"));
            std::list<ColormapWithModifiers> names;
            if (auto filename = args.try_named_value("--kn5"); filename != nullptr) {
                auto kn5 = load_kn5(*filename);
                for (auto& [name, data] : kn5.textures) {
                    if (!Mlib::u32regex_search(*name, re)) {
                        continue;
                    }
                    linfo() << "Matched: " << *name;
                    auto cm = ColormapWithModifiers{
                        .filename = FPath::from_variable_and_hash(name),
                        .color_mode = ColorMode::RGBA,
                        .mipmap_mode = MipmapMode::WITH_MIPMAPS
                    }.compute_hash();
                    names.push_back(cm);
                    rendering_resources.add_texture(
                        cm,
                        std::move(data.data),
                        FlipMode::VERTICAL,
                        TextureAlreadyExistsBehavior::RAISE);
                }
            }
            if (auto filename = args.try_named_value("--txd"); filename != nullptr) {
                auto txd = Dff::read_txd(
                    *filename,
                    rvalue_address(Dff::RasterFactory()),
                    rvalue_address(Dff::RasterConfig{
                        .need_to_read_back_textures = true,
                        .make_native = true,
                        .flip_gl_y_axis = true}),
                    IoVerbosity::SILENT);
                for (auto& tx : txd.textures) {
                    if (!Mlib::u32regex_search(*tx->name, re)) {
                        linfo() << "Skipping: " << *tx->name;
                        continue;
                    }
                    linfo() << "Matched: " << *tx->name;
                    auto cm = ColormapWithModifiers{
                            .filename = FPath::from_variable_and_hash(tx->name),
                            .color_mode = ColorMode::RGBA,
                            .mipmap_mode = MipmapMode::WITH_MIPMAPS
                        }.compute_hash();
                    names.push_back(cm);
                    TextureSize size{
                        .width = integral_cast<int>(tx->raster->width()),
                        .height = integral_cast<int>(tx->raster->height())
                    };
                    rendering_resources.set_texture(
                        cm,
                        tx->raster->texture_handle(),
                        &size);
                }
            }
            if (names.empty()) {
                throw std::runtime_error("Could not find a single texture matching \"" + U8::str(args.named_value("--filter")) + '"');
            }
            rendering_resources.generate_auto_texture_atlas(
                &atlas,
                ColormapWithModifiers{
                    .filename = FPath::from_variable_and_hash(tmp_texture),
                    .color_mode = ColorMode::RGBA,
                    .anisotropic_filtering_level = safe_stou(args.named_svalue("--aniso", "0"))
                }.compute_hash(),
                std::vector(names.begin(), names.end()),
                safe_stoi(args.named_svalue("--mip_level_count")),
                safe_stoi(args.named_svalue("--size", "4096")));
            for (const auto& [i, t] : enumerate(atlas.tiles)) {
                linfo() << "Layer: " << i;
                for (const auto& d : t) {
                    linfo() << "  Filename: " << (const std::string&)d.name.filename << ' ' << d.width << 'x' << d.height;
                }
            }
            if (auto filename = args.try_named_value("--texname"); filename != nullptr) {
                ftl.emplace(
                    rendering_resources.get_texture(ColormapWithModifiers{
                        .filename = FPath::from_variable(U8::str(*filename)),
                        .color_mode = ColorMode::RGBA,
                        .mipmap_mode = MipmapMode::WITH_MIPMAPS
                    }.compute_hash()));
            } else if (!args.has_named("--rerender_atlas")) {
                auto layer = safe_stoz(args.named_svalue("--atlas_layer"));
                if (layer >= atlas.tiles.size()) {
                    throw std::runtime_error("Layer index out of bounds");
                }
                ftl.emplace(
                    rendering_resources.get_texture(
                        ColormapWithModifiers{
                            .filename = FPath::from_variable_and_hash(tmp_texture),
                            .color_mode = ColorMode::RGBA,
                            .mipmap_mode = MipmapMode::WITH_MIPMAPS}.compute_hash()),
                    CullFaceMode::CULL,
                    ContinuousBlendMode::ALPHA,
                    standard_quad_vertices,
                    layer);
                rendering_resources.preload({
                    .color = {
                        .filename = FPath::from_variable_and_hash(tmp_texture),
                        .color_mode = ColorMode::RGBA,
                        .mipmap_mode = MipmapMode::WITH_MIPMAPS
                    }});
            }
        } else if (unnamed.size() == 1) {
            rendering_resources.add_texture_descriptor(
                tmp_texture,
                TextureDescriptor{
                    .color = {
                        .filename = FPath::from_variable(U8::str(args.unnamed_value(0))),
                        .color_mode = ColorMode::RGBA,
                        .mipmap_mode = MipmapMode::WITH_MIPMAPS}});
            ftl.emplace(
                rendering_resources.get_texture(ColormapWithModifiers{
                    .filename = FPath::from_variable_and_hash(tmp_texture),
                    .color_mode = ColorMode::RGBA,
                    .mipmap_mode = MipmapMode::WITH_MIPMAPS}.compute_hash()));
        } else {
            throw std::runtime_error("Expected 0 or 1 unnamed arguments");
        }

        // render loop
        // -----------
        while (!window.close_requested())
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
                ftl->render(ClearMode::OFF);
            } else if (!atlas.tiles.empty()) {
                auto layer = safe_stoz(args.named_svalue("--atlas_layer"));
                if (layer >= atlas.tiles.size()) {
                    throw std::runtime_error("Layer index out of bounds");
                }
                float scale = std::pow(2.f, -safe_stof(args.named_svalue("--mip_level")));
                if (std::isnan(scale)) {
                    throw std::runtime_error("Scale is NAN");
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
    return 0;
}
