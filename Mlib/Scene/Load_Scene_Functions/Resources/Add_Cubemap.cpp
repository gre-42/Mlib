#include "Add_Cubemap.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Geometry/Material/Texture_Descriptor.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Resource_Managers/Rendering_Resources.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(alias);
DECLARE_ARGUMENT(filenames);
}

const std::string AddCubemap::key = "add_cubemap";

LoadSceneJsonUserFunction AddCubemap::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    execute(args);
};

void AddCubemap::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    RenderingContextStack::primary_rendering_resources().add_cubemap(
        args.arguments.at<VariableAndHash<std::string>>(KnownArgs::alias),
        args.arguments.pathes_or_variables(KnownArgs::filenames, [](const FPath& p) { return VariableAndHash{ p.path }; }));
}
