#include "Deallocators.hpp"
#include <Mlib/Memory/Deallocation_Token.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

Deallocators::Deallocators() = default;

Deallocators::~Deallocators() = default;

DeallocationToken Deallocators::insert(const std::function<void()>& deallocate) {
    std::scoped_lock lock{ mutex_ };
    deallocators_.push_front(deallocate);
    return { this, deallocators_.begin() };
}

void Deallocators::erase(const std::list<std::function<void()>>::iterator& token) {
    std::scoped_lock lock{ mutex_ };
    deallocators_.erase(token);
}

void Deallocators::deallocate() {
    std::scoped_lock lock{ mutex_ };
    for (auto& r : deallocators_) {
        r();
    }
}
