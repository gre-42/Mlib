#include "Deallocation_Token.hpp"
#include <Mlib/Memory/Deallocators.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

DeallocationToken DeallocationToken::empty() noexcept {
    return DeallocationToken{nullptr, std::list<std::function<void()>>::iterator()};
}

DeallocationToken::DeallocationToken(DeallocationToken&& other) noexcept
: deallocators_{other.deallocators_},
  it_{std::move(other.it_)}
{
    other.deallocators_ = nullptr;
}

DeallocationToken& DeallocationToken::operator = (DeallocationToken&& other) {
    if (deallocators_ != nullptr) {
        THROW_OR_ABORT("Deallocators already set");
    }
    deallocators_ = other.deallocators_;
    it_ = std::move(other.it_);
    other.deallocators_ = nullptr;
    return *this;
}


DeallocationToken::DeallocationToken(
    Deallocators* deallocators,
    const std::list<std::function<void()>>::iterator& it) noexcept
: deallocators_{deallocators},
  it_{it}
{ }

DeallocationToken::~DeallocationToken() {
    if (deallocators_ != nullptr) {
        deallocators_->erase(it_);
    }
}
