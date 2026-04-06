#include "Facade_Texture.hpp"
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Strings/String_View_To_Number.hpp>
#include <stdexcept>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(selector);
DECLARE_ARGUMENT(facade);
DECLARE_ARGUMENT(material);
DECLARE_ARGUMENT(min_height);
DECLARE_ARGUMENT(max_height);
DECLARE_ARGUMENT(interior);
}

namespace InteriorArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(facade_edge_size);
DECLARE_ARGUMENT(facade_inner_size);
DECLARE_ARGUMENT(interior_size);
DECLARE_ARGUMENT(left);
DECLARE_ARGUMENT(right);
DECLARE_ARGUMENT(floor);
DECLARE_ARGUMENT(ceiling);
DECLARE_ARGUMENT(back);
DECLARE_ARGUMENT(back_specular);
DECLARE_ARGUMENT(front);
DECLARE_ARGUMENT(front_alpha);
DECLARE_ARGUMENT(front_specular);
}

FacadeTexture Mlib::parse_facade_texture(const JsonMacroArguments& args) {
    args.validate(KnownArgs::options, "facade texture: ");

    InteriorTextures itx;
    auto interior = args.try_get_child(KnownArgs::interior);
    if (interior.has_value()) {
        const auto& i = interior.value();
        i.validate(InteriorArgs::options);
        itx.facade_edge_size = i.at<EOrderableFixedArray<float, 2>>(InteriorArgs::facade_edge_size);
        itx.facade_inner_size = i.at<EOrderableFixedArray<float, 2>>(InteriorArgs::facade_inner_size);
        itx.interior_size = i.at<EOrderableFixedArray<float, 3>>(InteriorArgs::interior_size);
        itx.assign(
            i.path_or_variable(InteriorArgs::left),
            i.path_or_variable(InteriorArgs::right),
            i.path_or_variable(InteriorArgs::floor),
            i.path_or_variable(InteriorArgs::ceiling),
            i.path_or_variable(InteriorArgs::back),
            i.try_path_or_variable(InteriorArgs::back_specular),
            i.try_path_or_variable(InteriorArgs::front),
            i.try_path_or_variable(InteriorArgs::front_alpha),
            i.try_path_or_variable(InteriorArgs::front_specular));
    }
    return FacadeTexture{
        .selector = args.at<std::string>(KnownArgs::selector, ""),
        .min_height = args.at<float>(KnownArgs::min_height, -INFINITY),
        .max_height = args.at<float>(KnownArgs::max_height, INFINITY),
        .descriptor = FacadeTextureDescriptor{
            .names = args.pathes_or_variables(KnownArgs::facade),
            .material = physics_material_from_string(args.at<std::string>(KnownArgs::material)),
            .interior_textures = std::move(itx)}};
}
