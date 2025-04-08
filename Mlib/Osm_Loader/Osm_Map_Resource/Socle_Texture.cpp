#include "Socle_Texture.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(textures);
DECLARE_ARGUMENT(material);
}

SocleTexture Mlib::parse_socle_texture(const JsonMacroArguments& args) {
    args.validate(KnownArgs::options, "socle texture: ");

    return {
        .textures = args.at<std::vector<VariableAndHash<std::string>>>(KnownArgs::textures),
        .material = physics_material_from_string(args.at<std::string>(KnownArgs::material))
    };
}
