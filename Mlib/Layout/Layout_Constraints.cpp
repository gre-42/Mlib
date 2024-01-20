#include "Layout_Constraints.hpp"
#include <Mlib/Layout/Concrete_Layout_Pixels.hpp>
#include <Mlib/Layout/Screen_Units.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <mutex>

using namespace Mlib;

LayoutConstraints::LayoutConstraints() {
    set_pixels("min", std::make_unique<MinimumConstraint>());
    set_pixels("end", std::make_unique<EndConstraint>());
}

LayoutConstraints::~LayoutConstraints() = default;

ILayoutPixels& LayoutConstraints::get_pixels(const std::string& name) const {
    std::shared_lock lock{mutex_};
    auto it = pixels_.find(name);
    if (it == pixels_.end()) {
        THROW_OR_ABORT("Could not find constraint with name \"" + name + '"');
    }
    return *it->second;
}

void LayoutConstraints::set_pixels(const std::string& name, std::unique_ptr<ILayoutPixels>&& constraint) {
    std::scoped_lock lock{mutex_};
    if (!pixels_.insert({name, std::move(constraint)}).second) {
        THROW_OR_ABORT("Constraint with name \"" + name + "\" already exists");
    }
}
