#include "Layout_Constraints.hpp"
#include <Mlib/Layout/Concrete_Layout_Constraints.hpp>
#include <Mlib/Layout/Screen_Units.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <mutex>

using namespace Mlib;

LayoutConstraints::LayoutConstraints() {
    set_scalar("min", std::make_unique<ConstantConstraint>(0.f, ScreenUnits::PIXELS));
    set_scalar("max", std::make_unique<MaximumConstraint>());
}

LayoutConstraints::~LayoutConstraints() = default;

ILayoutScalar& LayoutConstraints::get_scalar(const std::string& name) const {
    std::shared_lock lock{mutex_};
    auto it = constraints_.find(name);
    if (it == constraints_.end()) {
        THROW_OR_ABORT("Could not find constraint with name \"" + name + '"');
    }
    return *it->second;
}

void LayoutConstraints::set_scalar(const std::string& name, std::unique_ptr<ILayoutScalar>&& constraint) {
    std::unique_lock lock{mutex_};
    if (!constraints_.insert({name, std::move(constraint)}).second) {
        THROW_OR_ABORT("Constraint with name \"" + name + "\" already exists");
    }
}
