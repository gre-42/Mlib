#pragma once
#include <Mlib/Array/Array.hpp>
#include <Mlib/Array/Sparse_Array.hpp>
#include <Mlib/Sfm/Marginalization/UUID.hpp>
#include <map>

namespace Mlib {

class MarginalizingBias {
public:
    MarginalizingBias(float alpha, float beta);
    void marginalize(
        const SparseArrayCcs<float>& J,
        const Array<float>& x0,
        const Array<size_t>& ids_a,
        const Array<size_t>& ids_b);
    void marginalize_wip(
        const SparseArrayCcs<float>& J,
        const Array<float>& x0,
        const Array<float>& residual,
        const Array<size_t>& ids_a,
        const Array<size_t>& ids_b);
    void update_indices(const std::map<UUID, size_t>& uuids_ka);
    float bias(const Array<float>& x) const;
    Array<float> solve(
        const SparseArrayCcs<float>& J,
        const Array<float>& x,
        const Array<float>& residual,
        const Array<size_t>& ids_a = Array<size_t>(),
        const Array<size_t>& ids_b = Array<size_t>()) const;
    Array<float> lhs_ka_;
    Array<float> rhs_ka_;
    std::map<size_t, UUID> uuids_ka_map_;
    ArrayShape shape_;
private:
    float alpha_;
    float beta_;
};

}
