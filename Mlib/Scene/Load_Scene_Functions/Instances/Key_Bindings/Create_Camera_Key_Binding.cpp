#include "Create_Camera_Key_Binding.hpp"
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Key_Bindings/Camera_Key_Binding.hpp>
#include <Mlib/Scene/Load_Scene_User_Function_Args.hpp>
#include <Mlib/Scene/Render_Logics/Key_Bindings.hpp>
#include <Mlib/Strings/To_Number.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(ID);

const std::string CreateCameraKeyBinding::key = "camera_key_binding";

LoadSceneUserFunction CreateCameraKeyBinding::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^id=(\\w+)$");
    Mlib::re::smatch match;
    if (!Mlib::re::regex_match(args.line, match, regex)) {
        THROW_OR_ABORT("Could not parse user function arguments");
    }
    CreateCameraKeyBinding(args.renderable_scene()).execute(match, args);
};

CreateCameraKeyBinding::CreateCameraKeyBinding(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateCameraKeyBinding::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    key_bindings.add_camera_key_binding({.id = match[ID].str()});
}
