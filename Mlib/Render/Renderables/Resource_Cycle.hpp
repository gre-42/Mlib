#pragma once
#include <Mlib/Stats/Fast_Random_Number_Generators.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <functional>
#include <vector>

namespace Mlib {

template <class TResource>
class ResourceCycle {
public:
    ResourceCycle(std::vector<TResource> resources)
    : index_{ 1, 0, std::max((size_t)1, resources.size()) - 1 },
      resources_{ std::move(resources) }
    {}
    ~ResourceCycle()
    {}
    template <class TPredicate>
    const TResource* try_once(const TPredicate& predicate) {
        if (resources_.empty()) {
            THROW_OR_ABORT("Resource cycle called with empty names");
        }
        const TResource& prn = resources_[index_()];
        return predicate(prn) ? &prn : nullptr;
    }
    template <class TPredicate>
    const TResource* try_multiple_times(const TPredicate& predicate, size_t nattempts) {
        for(size_t attempt = 0; attempt < nattempts; ++attempt) {
            const TResource* res = try_once(predicate);
            if (res != nullptr) {
                return res;
            }
        }
        return nullptr;
    }
    template <class TPredicate>
    const TResource& operator () (const TPredicate& predicate) {
        const TResource* result = try_multiple_times(predicate, 100);
        if (result == nullptr) {
            THROW_OR_ABORT("Could not find resource after 100 attempts");
        }
        return *result;
    }
    bool empty() const {
        return resources_.empty();
    }
    void seed(unsigned int seed) {
        index_.seed(seed);
    }
    decltype(auto) begin() const {
        return resources_.begin();
    }
    decltype(auto) end() const {
        return resources_.end();
    }
private:
    FastUniformIntRandomNumberGenerator<size_t> index_;
    std::vector<TResource> resources_;
};

}
