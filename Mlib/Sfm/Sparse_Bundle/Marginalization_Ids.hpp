#pragma once
#include <Mlib/Array/Array.hpp>
#include <chrono>
#include <set>

namespace Mlib { namespace Sfm {

class GlobalBundle;

class MarginalizationIds {
public:
    explicit MarginalizationIds(const GlobalBundle& gb);

    void linearize_point(size_t index);
    void linearize_camera(const std::chrono::milliseconds& time);
    void marginalize_point(size_t index);
    void marginalize_camera(const std::chrono::milliseconds& time);

    Array<size_t> ids_a;
    Array<size_t> ids_b;

private:
    const GlobalBundle& gb_;
    std::set<size_t> ids_;

};

}}
