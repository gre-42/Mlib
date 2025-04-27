#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Geometry/Material/Texture_Descriptor.hpp>
#include <Mlib/Geometry/Mesh/Load/Load_Kn5.hpp>
#include <Mlib/Geometry/Mesh/Load/Save_Kn5.hpp>
#include <Mlib/Memory/Destruction_Guard.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Gl_Context_Guard.hpp>
#include <Mlib/Render/Resource_Managers/Rendering_Resources.hpp>
#include <Mlib/Render/Window.hpp>
#include <Mlib/Threads/Realtime_Threads.hpp>
#include <stb/stb_image_write.h>
#include <stb_cpp/stb_encode.hpp>
#include <stb_cpp/stb_image_load.hpp>

using namespace Mlib;

int main(int argc, char** argv) {

    const ArgParser parser(
        "Usage: repackage_kn5 <source> <destination> [--explode]",
        {"--explode"},
        {});
    try {
        const auto args = parser.parsed(argc, argv);

        args.assert_num_unnamed(2);

        reserve_realtime_threads(0);

        // glfw: initialize and configure
        // ------------------------------
        GLFW_CHK(glfwInit());
        DestructionGuard dg{[](){glfwTerminate();}};
        GLFW_CHK(glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4));
        GLFW_CHK(glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1));
        GLFW_CHK(glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE));
        GLFW_CHK(glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE));

#ifdef __APPLE__
        GLFW_CHK(glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE));
#endif

        // glfw window creation
        // --------------------
        Window window{
            800,
            600,
            "Repackage kn5",
            nullptr,    // monitor
            nullptr,    // share
            true,       // use_double_buffering
            1,          // swap_interval
            0};         // fullscreen_refresh_rate
        GlContextGuard gcg{ window };
        if (int version = gladLoadGL(glfwGetProcAddress); version == 0) {
            THROW_OR_ABORT("gladLoadGL failed");
        }

        // Resources
        // ---------
        RenderingResources rendering_resources{
            "primary resources",
            0 };  // max_anisotropic_filter_level 

        auto gen_png_name = [](const VariableAndHash<std::string>& dds_name)
            { return VariableAndHash{ *dds_name + ".png" }; };
        auto gen_optional_png_name = [&gen_png_name](const VariableAndHash<std::string>& dds_name)
            { return dds_name->empty() ? VariableAndHash<std::string>{} : gen_png_name(dds_name); };
        const auto& source_filename = args.unnamed_value(0);
        auto source = create_ifstream(source_filename, std::ios::binary);
        if (source->fail()) {
            THROW_OR_ABORT("Could not open file for read: \"" + source_filename + '"');
        }
        auto kn5 = load_kn5(*source, false /* verbose */, kn5LoadOptions::MATERIALS);
        std::map<std::string, kn5Texture> destination_textures;
        for (auto& [dds_name, t] : kn5.textures) {
            auto png_name = gen_png_name(dds_name);
            auto colormap = ColormapWithModifiers{
                .filename = dds_name,
                .color_mode = ColorMode::RGBA,
                .mipmap_mode = MipmapMode::WITH_MIPMAPS
            }.compute_hash();
            rendering_resources.add_texture(colormap, std::move(t.data), FlipMode::VERTICAL, TextureAlreadyExistsBehavior::RAISE);
            auto tex = rendering_resources.get_texture_data(
                colormap,
                TextureRole::COLOR,
                FlipMode::NONE);
            auto& dest = destination_textures[*png_name];
            dest.type = t.type;
            dest.data = stb_encode_png(tex->data(), tex->width, tex->height, tex->nrChannels);
            if (args.has_named("--explode")) {
                auto o = create_ofstream(*png_name, std::ios::binary);
                if (o->fail()) {
                    THROW_OR_ABORT("Could not open \"" + *png_name + '"');
                }
                o->write((const char*)dest.data.data(), integral_cast<std::streamsize>(dest.data.size()));
                o->flush();
                if (o->fail()) {
                    THROW_OR_ABORT("Could not write to \"" + *png_name + '"');
                }
            }
        }
        std::map<size_t, kn5Material> destination_materials;
        for (auto& [material_name, dds_material] : kn5.materials) {
            auto& destination_material = destination_materials[material_name];
            destination_material = dds_material;
            destination_material.txDiffuse = gen_optional_png_name(dds_material.txDiffuse);
            destination_material.txNormal = gen_optional_png_name(dds_material.txNormal);
            destination_material.txMask = gen_optional_png_name(dds_material.txMask);
            destination_material.txDetail4(0) = gen_optional_png_name(dds_material.txDetail4(0));
            destination_material.txDetail4(1) = gen_optional_png_name(dds_material.txDetail4(1));
            destination_material.txDetail4(2) = gen_optional_png_name(dds_material.txDetail4(2));
            destination_material.txDetail4(3) = gen_optional_png_name(dds_material.txDetail4(3));
            destination_material.txDetail1 = gen_optional_png_name(dds_material.txDetail1);
            destination_material.txDetailNM = gen_optional_png_name(dds_material.txDetailNM);
            destination_material.txVariation = gen_optional_png_name(dds_material.txVariation);
        }
        save_kn5(args.unnamed_value(1), kn5.version, kn5.unknownNo, destination_textures, destination_materials, *source);
    } catch (const std::exception& e) {
        lerr() << e.what();
        return 1;
    }
    return 0;
}
