#include "Deallocation_Token.hpp"
#include <Mlib/Deallocators.hpp>

using namespace Mlib;

DeallocationToken::DeallocationToken(DeallocationToken&& other) noexcept
: deallocators_{other.deallocators_},
  it_{other.it_}
{
    other.deallocators_ = nullptr;
}

DeallocationToken::DeallocationToken(
    Deallocators& deallocators,
    const std::list<std::function<void()>>::iterator& it)
: deallocators_{&deallocators},
  it_{it}
{ }

DeallocationToken::~DeallocationToken() {
    if (deallocators_ != nullptr) {
        deallocators_->erase(it_);
    }
}
