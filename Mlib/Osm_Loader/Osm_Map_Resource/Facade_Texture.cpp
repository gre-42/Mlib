#include "Facade_Texture.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Strings/To_Number.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(selector);
DECLARE_ARGUMENT(facade);
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
}

FacadeTexture Mlib::parse_facade_texture(const JsonMacroArguments& args) {
    args.validate(KnownArgs::options, "facade texture: ");

    InteriorTextures itx;
    auto interior = args.try_get_child(KnownArgs::interior);
    if (interior.has_value()) {
        interior.value().validate(InteriorArgs::options);
        itx = InteriorTextures{
            .facade_edge_size = interior.value().at<OrderableFixedArray<float, 2>>(InteriorArgs::facade_edge_size),
            .facade_inner_size = interior.value().at<OrderableFixedArray<float, 2>>(InteriorArgs::facade_inner_size),
            .interior_size = interior.value().at<OrderableFixedArray<float, 3>>(InteriorArgs::interior_size),
            .left = interior.value().at<std::string>(InteriorArgs::left),
            .right = interior.value().at<std::string>(InteriorArgs::right),
            .floor = interior.value().at<std::string>(InteriorArgs::floor),
            .ceiling = interior.value().at<std::string>(InteriorArgs::ceiling),
            .back = interior.value().at<std::string>(InteriorArgs::back)
        };
    }
    return FacadeTexture{
        .selector = args.at<std::string>(KnownArgs::selector, ""),
        .min_height = args.at<float>(KnownArgs::min_height, -INFINITY),
        .max_height = args.at<float>(KnownArgs::max_height, INFINITY),
        .descriptor = FacadeTextureDescriptor{
            .name = args.at<std::string>(KnownArgs::facade),
            .interior_textures = itx}};
}
