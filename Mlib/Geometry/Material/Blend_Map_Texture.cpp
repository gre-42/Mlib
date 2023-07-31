#include "Blend_Map_Texture.hpp"
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

BlendMapRole Mlib::blend_map_role_from_string(const std::string& s) {
    if (s == "summand") {
        return BlendMapRole::SUMMAND;
    } else if (s == "detail_base") {
        return BlendMapRole::DETAIL_BASE;
    } else if (s == "detail_mask_r") {
        return BlendMapRole::DETAIL_MASK_R;
    } else if (s == "detail_mask_g") {
        return BlendMapRole::DETAIL_MASK_G;
    } else if (s == "detail_mask_b") {
        return BlendMapRole::DETAIL_MASK_B;
    } else if (s == "detail_mask_a") {
        return BlendMapRole::DETAIL_MASK_A;
    } else if (s == "detail_color") {
        return BlendMapRole::DETAIL_COLOR;
    }
    THROW_OR_ABORT("Unknown blend map role: \"" + s + '"');
}
