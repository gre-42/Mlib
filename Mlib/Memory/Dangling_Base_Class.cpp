#include "Dangling_Base_Class.hpp"
#include <mutex>

using namespace Mlib;

DanglingBaseClass::DanglingBaseClass() = default;

DanglingBaseClass::~DanglingBaseClass() {
    if (!locs_.empty()) {
        lerr() << "Remaining locations: " << locs_.size();
        for (const auto& [p, l] : locs_) {
            lerr() << l.file_name() << ':' << l.line();
        }
        verbose_abort("Dangling pointers or references remaining");
    }
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
