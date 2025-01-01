#include "Add_To_Gallery.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Geometry/Material/Color_Mode.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Render/Render_Logic_Gallery.hpp>
#include <Mlib/Render/Render_Logics/Fill_With_Texture_Logic.hpp>
#include <Mlib/Render/Render_Logics/Resource_Update_Cycle.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Resource_Managers/Rendering_Resources.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>

using namespace Mlib;

const std::string AddToGallery::key = "add_to_gallery";

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(resource);
DECLARE_ARGUMENT(instance);
DECLARE_ARGUMENT(color_mode);
DECLARE_ARGUMENT(flip_horizontally);
}

LoadSceneJsonUserFunction AddToGallery::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    args.gallery.insert(
        args.arguments.at<std::string>(KnownArgs::instance),
        std::make_unique<FillWithTextureLogic>(
            RenderingContextStack::primary_rendering_resources().get_texture_lazy(
                ColormapWithModifiers{
                    .filename = VariableAndHash{ args.arguments.path_or_variable(KnownArgs::resource).path },
                    .color_mode = color_mode_from_string(args.arguments.at<std::string>(KnownArgs::color_mode)),
                    .mipmap_mode = MipmapMode::WITH_MIPMAPS
                }.compute_hash(),
                TextureRole::COLOR_FROM_DB),
            CullFaceMode::CULL,
            ContinuousBlendMode::ALPHA,
            args.arguments.at<bool>(KnownArgs::flip_horizontally, false)
                ? horizontally_flipped_quad_vertices
                : standard_quad_vertices));
};
