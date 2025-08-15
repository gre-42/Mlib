#include "Renderable_With_Style.hpp"
#include <Mlib/Hash.hpp>
#include <shared_mutex>

using namespace Mlib;

static const size_t SEED = 0xc0febabe + 1;

RenderableWithStyle::RenderableWithStyle(std::shared_ptr<const Renderable> renderable)
    : renderable_{ std::move(renderable) }
    , style_hash_{ SEED }
{ }

RenderableWithStyle::~RenderableWithStyle() = default;

const ColorStyle* RenderableWithStyle::style(
    const std::list<const ColorStyle*>& color_styles,
    const VariableAndHash<std::string>& name) const
{
    Hasher hasher0{ SEED };
    for (const auto* style : color_styles) {
        if (style->matches(name)) {
            hasher0.combine(style->get_hash());
        }
    }
    if (hasher0 == SEED) {
        return nullptr;
    }
    {
        std::shared_lock lock{ style_hash_mutex_ };
        if (hasher0 == style_hash_) {
            if (!style_.has_value()) {
                THROW_OR_ABORT("Style hash collision (0)");
            }
            return &*style_;
        }
    }
    {
        std::scoped_lock lock{ style_hash_mutex_ };
        if (hasher0 == style_hash_) {
            if (!style_.has_value()) {
                THROW_OR_ABORT("Style hash collision (1)");
            }
            return &*style_;
        }
        style_.reset();
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
        style_.emplace();
#pragma GCC diagnostic pop
        Hasher hasher1{ SEED };
        for (const auto& style : color_styles) {
            if (style->matches(name)) {
                std::scoped_lock lock{style->hash_mutex_};
                hasher1.combine(style->hash_.get());
                style_->insert(*style);
            }
        }
        style_hash_ = hasher1;
    }
    return &*style_;
}
