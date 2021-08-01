#pragma once
#include <Mlib/Cpx/Tag.hpp>
#include <Mlib/Cpx/Text.hpp>
#include <functional>
#include <memory>

namespace Mlib::cpx {

// Tags
struct html { static const char *name() { return "html"; } };
struct head { static const char *name() { return "head"; } };
struct meta { static const char *name() { return "meta"; } };
struct body { static const char *name() { return "body"; } };
struct span { static const char *name() { return "span"; } };
struct div_ { static const char *name() { return "div"; } };

// Underscore
template <class TTagName>
std::shared_ptr<Tag> _(std::initializer_list<std::shared_ptr<Renderable>> contents...) {
    std::list<std::shared_ptr<Renderable>> renderables(contents);
    return std::make_shared<Tag>(TTagName::name(), renderables);
}

template <class TTagName>
std::shared_ptr<Tag> _(const std::string& contents) {
    std::list<std::shared_ptr<Renderable>> renderables{std::make_shared<Text>(contents)};
    return std::make_shared<Tag>(TTagName::name(), renderables);
}

template <class TTagName>
std::shared_ptr<Tag> _(const std::list<std::shared_ptr<Renderable>>& renderables) {
    return std::make_shared<Tag>(TTagName::name(), renderables);
}

// Apply
template <class TIterable, class TCallable>
std::list<std::shared_ptr<Renderable>> gen_renderables(
    const TIterable& iterable,
    const TCallable& callback)
{
    std::list<std::shared_ptr<Renderable>> result;
    for (const auto& in : iterable) {
        result.push_back(callback(in));
    }
    return result;
}

}
