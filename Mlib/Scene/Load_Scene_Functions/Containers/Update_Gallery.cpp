#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <stdexcept>
#ifndef WITHOUT_GRAPHICS
#include <Mlib/OpenGL/Render_Logic_Gallery.hpp>
#include <Mlib/OpenGL/Render_Logics/Fill_With_Texture_Logic.hpp>
#include <Mlib/Resource_Context/Rendering_Context.hpp>
#include <Mlib/OpenGL/Resource_Managers/Rendering_Resources.hpp>
#endif

using namespace Mlib;

namespace {

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(resource);
DECLARE_ARGUMENT(instance);
DECLARE_ARGUMENT(color_mode);
}

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "update_gallery",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                args.arguments.validate(KnownArgs::options);
#ifndef WITHOUT_GRAPHICS
                auto entry = args.gallery[args.arguments.at(KnownArgs::instance)];
                entry->set_image_resource_name(
                    RenderingContextStack::primary_rendering_resources().get_texture_lazy(
                        ColormapWithModifiers{
                            .filename = args.arguments.path_or_variable(KnownArgs::resource),
                            .color_mode = color_mode_from_string(args.arguments.at<std::string>(KnownArgs::color_mode)),
                            .mipmap_mode = MipmapMode::WITH_MIPMAPS
                        }.compute_hash(),
                        TextureRole::COLOR_FROM_DB));
#endif
            });
    }
} obj;

}
