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
    } else if (s == "detail_color"sv) {
        return BlendMapRole::DETAIL_COLOR;
    }
    THROW_OR_ABORT("Unknown blend map role: \"" + std::string{ s } + '"');
}

BlendMapUvSource Mlib::blend_map_uv_source_from_string(std::string_view s) {
    if (s == "vertical"sv) {
        return BlendMapUvSource::VERTICAL;
    } else if (s == "horizontal"sv) {
        return BlendMapUvSource::HORIZONTAL;
    }
    THROW_OR_ABORT("Unknown UV source: \"" + std::string{ s } + '"');
}

BlendMapReductionOperation Mlib::blend_map_reduction_operation_from_string(std::string_view s) {
    if (s == "plus"sv) {
        return BlendMapReductionOperation::PLUS;
    } else if (s == "minus"sv) {
        return BlendMapReductionOperation::MINUS;
    } else if (s == "times"sv) {
        return BlendMapReductionOperation::TIMES;
    } else {
        THROW_OR_ABORT("Unknown blend map reduction operation: \"" + std::string{ s } + '"');
    }
}
