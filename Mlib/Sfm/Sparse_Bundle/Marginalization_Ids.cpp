#include "Marginalization_Ids.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Sfm/Sparse_Bundle/Global_Bundle.hpp>

using namespace Mlib;
using namespace Mlib::Sfm;

MarginalizationIds::MarginalizationIds(const GlobalBundle& gb)
: ids_a{ArrayShape{0}},
  ids_b{ArrayShape{0}},
  gb_{gb}
{}

void MarginalizationIds::linearize_point(size_t index) {
    for (const auto& p : gb_.xp_uuids_.at(index).flat_iterable()) {
        size_t id = gb_.predictor_uuids_.at(p);
        if (ids_.find(id) != ids_.end()) {
            throw std::runtime_error("Duplicate marginalization-ID");
        }
        ids_a.append(id);
    }
}

void MarginalizationIds::linearize_camera(const std::chrono::milliseconds& time) {
    for (const auto& p : gb_.xk_uuids_.at(time).flat_iterable()) {
        size_t id = gb_.predictor_uuids_.at(p);
        if (ids_.find(id) != ids_.end()) {
            throw std::runtime_error("Duplicate marginalization-ID");
        }
        ids_a.append(id);
    }
}

void MarginalizationIds::marginalize_point(size_t index) {
    for (const auto& p : gb_.xp_uuids_.at(index).flat_iterable()) {
        size_t id = gb_.predictor_uuids_.at(p);
        if (ids_.find(id) != ids_.end()) {
            throw std::runtime_error("Duplicate marginalization-ID");
        }
        ids_b.append(id);
    }
}

void MarginalizationIds::marginalize_camera(const std::chrono::milliseconds& time) {
    for (const auto& p : gb_.xk_uuids_.at(time).flat_iterable()) {
        size_t id = gb_.predictor_uuids_.at(p);
        if (ids_.find(id) != ids_.end()) {
            throw std::runtime_error("Duplicate marginalization-ID");
        }
        ids_b.append(id);
    }
}
