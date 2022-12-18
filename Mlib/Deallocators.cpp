#include "Deallocators.hpp"
#include <Mlib/Deallocation_Token.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

DeallocationToken Deallocators::insert(const std::function<void()>& deallocate) {
    std::lock_guard lock{mutex_};
    deallocators_.push_front(deallocate);
    return DeallocationToken{*this, deallocators_.begin()};
}

void Deallocators::erase(const std::list<std::function<void()>>::iterator& token) {
    std::lock_guard lock{mutex_};
    deallocators_.erase(token);
}

void Deallocators::deallocate() {
    std::lock_guard lock{mutex_};
    for (auto& r : deallocators_) {
        r();
    }
}
