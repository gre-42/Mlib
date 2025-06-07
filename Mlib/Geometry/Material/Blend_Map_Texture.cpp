#include "Blend_Map_Texture.hpp"
#include <Mlib/Math/Orderable_Fixed_Array_Hash.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace std::string_view_literals;
using namespace Mlib;

BlendMapRole Mlib::blend_map_role_from_string(std::string_view s) {
    static const std::unordered_map<std::string_view, BlendMapRole> m{
        {"summand"sv, BlendMapRole::SUMMAND},
        {"detail_base"sv, BlendMapRole::DETAIL_BASE},
        {"detail_mask_r"sv, BlendMapRole::DETAIL_MASK_R},
        {"detail_mask_g"sv, BlendMapRole::DETAIL_MASK_G},
        {"detail_mask_b"sv, BlendMapRole::DETAIL_MASK_B},
        {"detail_mask_a"sv, BlendMapRole::DETAIL_MASK_A},
        {"detail_color"sv, BlendMapRole::DETAIL_COLOR}};
    auto it = m.find(s);
    if (it == m.end()) {
        THROW_OR_ABORT("Unknown blend map role: \"" + std::string{ s } + '"');
    }
    return it->second;
}

BlendMapUvSource Mlib::blend_map_uv_source_from_string(std::string_view s) {
    static_assert(BlendMapUvSource::VERTICAL_LAST == BlendMapUvSource::VERTICAL4);
    static const std::unordered_map<std::string_view, BlendMapUvSource> m{
        {"vertical0"sv, BlendMapUvSource::VERTICAL0},
        {"vertical1"sv, BlendMapUvSource::VERTICAL1},
        {"vertical2"sv, BlendMapUvSource::VERTICAL2},
        {"vertical3"sv, BlendMapUvSource::VERTICAL3},
        {"vertical4"sv, BlendMapUvSource::VERTICAL4},
        {"horizontal_xz"sv, BlendMapUvSource::HORIZONTAL_XZ},
        {"horizontal_xy"sv, BlendMapUvSource::HORIZONTAL_XY}
    };
    auto it = m.find(s);
    if (it == m.end()) {
        THROW_OR_ABORT("Unknown UV source: \"" + std::string{ s } + '"');
    }
    return it->second;
}

BlendMapReductionOperation Mlib::blend_map_reduction_operation_from_string(std::string_view s) {
    static const std::unordered_map<std::string_view, BlendMapReductionOperation> m{
        {"plus"sv, BlendMapReductionOperation::PLUS},
        {"minus"sv, BlendMapReductionOperation::MINUS},
        {"times"sv, BlendMapReductionOperation::TIMES},
        {"feather"sv, BlendMapReductionOperation::FEATHER},
        {"invert"sv, BlendMapReductionOperation::INVERT},
        {"blend"sv, BlendMapReductionOperation::BLEND},
        {"replace_color"sv, BlendMapReductionOperation::REPLACE_COLOR},
        {"colorize"sv, BlendMapReductionOperation::COLORIZE}
    };
    auto it = m.find(s);
    if (it == m.end()) {
        THROW_OR_ABORT("Unknown blend map reduction operation: \"" + std::string{ s } + '"');
    }
    return it->second;
}

BlendMapReweightMode Mlib::blend_map_reweight_mode_from_string(std::string_view s) {
    static const std::unordered_map<std::string_view, BlendMapReweightMode> m{
        {"undefined"sv, BlendMapReweightMode::UNDEFINED},
        {"enabled"sv, BlendMapReweightMode::ENABLED},
        {"disabled"sv, BlendMapReweightMode::DISABLED}
    };
    auto it = m.find(s);
    if (it == m.end()) {
        THROW_OR_ABORT("Unknown blend map reweight mode: \"" + std::string{ s } + '"');
    }
    return it->second;
}

size_t BlendMapTexture::modifiers_hash() const {
    return hash_combine(
        texture_descriptor.color.filename->empty(),
        texture_descriptor.specular.filename->empty(),
        texture_descriptor.normal.filename->empty(),
        texture_descriptor.color.color_mode,
        texture_descriptor.specular.color_mode,
        texture_descriptor.normal.color_mode,
        min_height,
        max_height,
        distances,
        normal,
        cosines,
        discreteness,
        offset,
        scale,
        weight,
        cweight_id,
        plus,
        min_detail_weight,
        role,
        uv_source,
        reduction,
        reweight_mode);
}
