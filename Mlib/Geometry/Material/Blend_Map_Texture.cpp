#include "Blend_Map_Texture.hpp"
#include <Mlib/Throw_Or_Abort.hpp>

using namespace std::string_view_literals;
using namespace Mlib;

BlendMapRole Mlib::blend_map_role_from_string(std::string_view s) {
    if (s == "summand"sv) {
        return BlendMapRole::SUMMAND;
    } else if (s == "detail_base"sv) {
        return BlendMapRole::DETAIL_BASE;
    } else if (s == "detail_mask_r"sv) {
        return BlendMapRole::DETAIL_MASK_R;
    } else if (s == "detail_mask_g"sv) {
        return BlendMapRole::DETAIL_MASK_G;
    } else if (s == "detail_mask_b"sv) {
        return BlendMapRole::DETAIL_MASK_B;
    } else if (s == "detail_mask_a"sv) {
        return BlendMapRole::DETAIL_MASK_A;
    } else if (s == "detail_color_horizontal"sv) {
        return BlendMapRole::DETAIL_COLOR_HORIZONTAL;
    } else if (s == "detail_color_vertical"sv) {
        return BlendMapRole::DETAIL_COLOR_VERTICAL;
    }
    THROW_OR_ABORT("Unknown blend map role: \"" + std::string{s} + '"');
}
