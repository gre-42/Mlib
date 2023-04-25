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
DECLARE_ARGUMENT(facade_edge_size_x);
DECLARE_ARGUMENT(facade_edge_size_y);
DECLARE_ARGUMENT(facade_inner_size_x);
DECLARE_ARGUMENT(facade_inner_size_y);
DECLARE_ARGUMENT(interior_size_x);
DECLARE_ARGUMENT(interior_size_y);
DECLARE_ARGUMENT(interior_size_z);
DECLARE_ARGUMENT(left);
DECLARE_ARGUMENT(right);
DECLARE_ARGUMENT(floor);
DECLARE_ARGUMENT(ceiling);
DECLARE_ARGUMENT(back);
}

FacadeTexture Mlib::parse_facade_texture(const JsonMacroArguments& args) {
    args.validate(KnownArgs::options);
    return FacadeTexture{
        .selector = args.at<std::string>(KnownArgs::selector),
        .min_height = args.at<float>(KnownArgs::min_height, -INFINITY),
        .max_height = args.at<float>(KnownArgs::max_height, INFINITY),
        .descriptor = FacadeTextureDescriptor{
            .name = args.at<std::string>(KnownArgs::facade),
            .interior_textures = InteriorTextures{
                .facade_edge_size = {
                    args.at<float>(KnownArgs::facade_edge_size_x, 0.f),
                    args.at<float>(KnownArgs::facade_edge_size_y, 0.f)},
                .facade_inner_size = {
                    args.at<float>(KnownArgs::facade_inner_size_x, 0.f),
                    args.at<float>(KnownArgs::facade_inner_size_y, 0.f)},
                .interior_size = {
                    args.at<float>(KnownArgs::interior_size_x, 0.f),
                    args.at<float>(KnownArgs::interior_size_y, 0.f),
                    args.at<float>(KnownArgs::interior_size_z, 0.f)},
                .left = args.at<std::string>(KnownArgs::left),
                .right = args.at<std::string>(KnownArgs::right),
                .floor = args.at<std::string>(KnownArgs::floor),
                .ceiling = args.at<std::string>(KnownArgs::ceiling),
                .back = args.at<std::string>(KnownArgs::back),
            }}};
}
