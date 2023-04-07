#include "Add_To_Gallery.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Geometry/Material/Color_Mode.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Render/Render_Logic_Gallery.hpp>
#include <Mlib/Render/Render_Logics/Fill_With_Texture_Logic.hpp>
#include <Mlib/Render/Render_Logics/Resource_Update_Cycle.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>

using namespace Mlib;

BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT_LIST(resource);
DECLARE_ARGUMENT_LIST(instance);
DECLARE_ARGUMENT_LIST(color_mode);
DECLARE_ARGUMENT_LIST(flip_horizontally);

LoadSceneJsonUserFunction AddToGallery::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    if (args.name == "add_to_gallery") {
        execute(args);
        return true;
    } else {
        return false;
    }
};

void AddToGallery::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(options);
    args.gallery.insert(
        args.arguments.get<std::string>(instance),
        std::make_unique<FillWithTextureLogic>(
            args.arguments.path(resource),
            ResourceUpdateCycle::ONCE,
            color_mode_from_string(args.arguments.get<std::string>(color_mode)),
            args.arguments.get<bool>(flip_horizontally, false)
                ? horizontally_flipped_quad_vertices
                : standard_quad_vertices));
}
