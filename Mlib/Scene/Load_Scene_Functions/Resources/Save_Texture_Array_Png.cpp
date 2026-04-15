#include "Save_Texture_Array_Png.hpp"
#include <Mlib/Geometry/Material/Texture_Descriptor.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/OpenGL/Rendering_Context.hpp>
#include <Mlib/OpenGL/Resource_Managers/Rendering_Resources.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <stb/stb_image_write.h>
#include <stb_cpp/stb_image_load.hpp>
#include <stdexcept>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(filename_prefix);
}

const std::string SaveTextureArrayPng::key = "save_texture_array_png";

LoadSceneJsonUserFunction SaveTextureArrayPng::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    execute(args);
};

void SaveTextureArrayPng::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    RenderingContextStack::primary_rendering_resources().save_array_to_file(
        args.arguments.path_or_variable(KnownArgs::filename_prefix).local_path(),
        RenderingContextStack::primary_rendering_resources().get_colormap(args.arguments.path_or_variable(KnownArgs::name)),
        TextureRole::COLOR);
}
