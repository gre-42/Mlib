#include "Layout_Constraints.hpp"
#include <Mlib/Layout/Concrete_Layout_Pixels.hpp>
#include <Mlib/Layout/Constraint_Window.hpp>
#include <Mlib/Layout/Screen_Units.hpp>
#include <Mlib/Layout/Widget.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <mutex>
#include <shared_mutex>

using namespace Mlib;

LayoutConstraints::LayoutConstraints() {
    set_pixels("min", std::make_unique<MinimumConstraint>());
    set_pixels("end", std::make_unique<EndConstraint>());
}

LayoutConstraints::~LayoutConstraints() = default;

ILayoutPixels& LayoutConstraints::get_pixels(const std::string& name) const {
    std::shared_lock lock{ mutex_ };
    auto it = pixels_.find(name);
    if (it == pixels_.end()) {
        THROW_OR_ABORT("Could not find constraint with name \"" + name + '"');
    }
    return *it->second;
}

std::unique_ptr<IWidget> LayoutConstraints::get_widget(const ConstraintWindow& window) const {
    return std::make_unique<Widget>(
        get_pixels(window.left),
        get_pixels(window.right),
        get_pixels(window.bottom),
        get_pixels(window.top));
}

void LayoutConstraints::set_pixels(std::string name, std::unique_ptr<ILayoutPixels>&& constraint) {
    std::scoped_lock lock{ mutex_ };
    if (!pixels_.insert({std::move(name), std::move(constraint)}).second) {
        THROW_OR_ABORT("Constraint with name \"" + name + "\" already exists");
    }
}
