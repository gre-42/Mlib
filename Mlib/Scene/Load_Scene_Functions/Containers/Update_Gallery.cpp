#include "Update_Gallery.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/FPath.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Render/Render_Logic_Gallery.hpp>
#include <Mlib/Render/Render_Logics/Fill_With_Texture_Logic.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Resource_Managers/Rendering_Resources.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(resource);
DECLARE_ARGUMENT(instance);
DECLARE_ARGUMENT(color_mode);
}

const std::string UpdateGallery::key = "update_gallery";

LoadSceneJsonUserFunction UpdateGallery::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    auto entry = args.gallery[args.arguments.at(KnownArgs::instance)];
    entry->set_image_resource_name(
        RenderingContextStack::primary_rendering_resources().get_texture_lazy(
            ColormapWithModifiers{
                .filename = VariableAndHash{ args.arguments.path_or_variable(KnownArgs::resource).path },
                .color_mode = color_mode_from_string(args.arguments.at<std::string>(KnownArgs::color_mode)),
                .mipmap_mode = MipmapMode::WITH_MIPMAPS
            }.compute_hash(),
            TextureRole::COLOR_FROM_DB));
};
