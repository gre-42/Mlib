#pragma once
#include <istream>
#include <memory>
#include <mutex>

namespace Mlib {

template <class TStreamOwner>
class IStreamAndLock: public std::istream {
public:
    explicit IStreamAndLock(
        std::istream& istr,
        std::unique_ptr<std::scoped_lock<std::recursive_mutex>>&& lock,
        const TStreamOwner& ref)
        : std::istream{ istr.rdbuf() }
        , lock_{ std::move(lock) }
        , ref_{ ref }
    {}
    virtual ~IStreamAndLock() override {
        ref_->reading_ = false;
    }
private:
    std::unique_ptr<std::scoped_lock<std::recursive_mutex>> lock_;
    TStreamOwner ref_;
};

}
