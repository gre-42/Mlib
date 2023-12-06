#include "Entrances_Texture.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(textures);
DECLARE_ARGUMENT(uv_scale_x);
}

EntrancesTexture Mlib::parse_entrances_texture(const JsonMacroArguments& args) {
    args.validate(KnownArgs::options, "entrances texture: ");

    return {
        .textures = args.at<std::vector<std::string>>(KnownArgs::textures),
        .uv_scale_x = args.at<float>(KnownArgs::uv_scale_x, 1.f)
    };
}
