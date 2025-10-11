#include "Dangling_Base_Class.hpp"
#include <mutex>

using namespace Mlib;

DanglingBaseClass::DanglingBaseClass() = default;

DanglingBaseClass::~DanglingBaseClass() {
    assert_no_references();
}

void DanglingBaseClass::print_references() const {
    lerr() << "Remaining locations: " << locs_.size();
    for (const auto& [p, l] : locs_) {
        lerr() << l;
    }
}

void DanglingBaseClass::assert_no_references() const {
    std::shared_lock lock{loc_mutex_};
    if (!locs_.empty()) {
        print_references();
        verbose_abort("Dangling pointers or references remaining");
    }
}

size_t DanglingBaseClass::nreferences() const {
    std::shared_lock lock{loc_mutex_};
    return locs_.size();
}

void DanglingBaseClass::add_source_location(const void* ptr, SourceLocation loc) {
    std::scoped_lock lock{loc_mutex_};
    if (!locs_.try_emplace(ptr, loc).second) {
        verbose_abort("Could not insert source location");
    }
}

void DanglingBaseClass::remove_source_location(const void* ptr) {
    std::scoped_lock lock{loc_mutex_};
    if (locs_.erase(ptr) != 1) {
        verbose_abort("Could not erase source location");
    }
}

const SourceLocation& DanglingBaseClass::loc(const void* ptr) const {
    std::shared_lock lock{loc_mutex_};
    auto it = locs_.find(ptr);
    if (it == locs_.end()) {
        verbose_abort("DanglingBaseClass::loc: Could not find location");
    }
    return it->second;
}
