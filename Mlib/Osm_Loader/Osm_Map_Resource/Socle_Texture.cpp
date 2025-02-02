#include "Socle_Texture.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(textures);
}

SocleTexture Mlib::parse_socle_texture(const JsonMacroArguments& args) {
    args.validate(KnownArgs::options, "socle texture: ");

    return {
        .textures = args.at<std::vector<VariableAndHash<std::string>>>(KnownArgs::textures)
    };
}
