#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Least_Squares/Rls.hpp>
#include <Mlib/Signal/Exponential_Smoother.hpp>
#include <iosfwd>

namespace Mlib {

class BandwidthEstimator {
public:
    BandwidthEstimator(
        float alpha = 0.95f,
        float lambda = 1e-7f,
        float header_size = 64.f);
    ~BandwidthEstimator();
    void update(float payload_size0, float payload_size1, float roundtrip_time);
    float latency() const;
    float bandwidth() const;
    float header_size() const;
    float dt(float payload_size) const;
    void print(std::ostream& ostr) const;
private:
    using Matrix = FixedArray<float, 2, 2>;
    using Vector = FixedArray<float, 2>;
    Rls<Matrix, Vector, float> rls_;
    float header_size_;
};

std::ostream& operator << (std::ostream& ostr, const BandwidthEstimator& be);

}
