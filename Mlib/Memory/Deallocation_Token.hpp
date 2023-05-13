#pragma once
#include <functional>
#include <list>

namespace Mlib {

class Deallocators;

class DeallocationToken {
    DeallocationToken(const DeallocationToken&) = delete;
    DeallocationToken& operator = (const DeallocationToken&) = delete;
public:
    explicit DeallocationToken(DeallocationToken&& other) noexcept;
    DeallocationToken(
        Deallocators& deallocators,
        const std::list<std::function<void()>>::iterator& it);
    ~DeallocationToken();
private:
    Deallocators* deallocators_;
    std::list<std::function<void()>>::iterator it_;
};

}
