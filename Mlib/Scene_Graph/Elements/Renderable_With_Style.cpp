#include "Renderable_With_Style.hpp"
#include <Mlib/Hash.hpp>
#include <shared_mutex>

using namespace Mlib;

static const size_t SEED = 0xc0febabe + 1;

RenderableWithStyle::RenderableWithStyle(std::shared_ptr<const Renderable> renderable)
    : renderable_{ std::move(renderable) }
    , style_hash_{ SEED }
{ }

const ColorStyle* RenderableWithStyle::style(
    const std::list<const ColorStyle*>& color_styles,
    const std::string& name) const
{
    Hasher hasher{ SEED };
    for (const auto* style : color_styles) {
        if (style->matches(name)) {
            hasher.combine(style);
        }
    }
    if (hasher == SEED) {
        return nullptr;
    }
    {
        std::shared_lock lock{ style_hash_mutex_ };
        if (hasher == style_hash_) {
            return &style_;
        }
    }
    {
        std::scoped_lock lock{ style_hash_mutex_ };
        if (hasher == style_hash_) {
            return &style_;
        }
        ColorStyle r_style;
        for (const auto& style : color_styles) {
            if (style->matches(name)) {
                r_style.insert(*style);
            }
        }
        style_ = r_style;
        style_hash_ = hasher;
    }
    return &style_;
}
