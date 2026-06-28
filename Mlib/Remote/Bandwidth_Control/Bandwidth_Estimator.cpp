#include "Bandwidth_Estimator.hpp"
#include <Mlib/Math/Fixed_Math.hpp>
#include <ostream>

using namespace Mlib;

BandwidthEstimator::BandwidthEstimator(
    float alpha,
    float lambda,
    float header_size)
    : rls_{fixed_scaled_diagonal_array<float, 2>(1 / lambda), fixed_zeros<float, 2>(), alpha}
    , header_size_{header_size}
{}

BandwidthEstimator::~BandwidthEstimator() = default;

void BandwidthEstimator::update(float payload_size0, float payload_size1, float roundtrip_time) {
    rls_.update(Vector{1.f, payload_size0 + payload_size1}, roundtrip_time);
}

float BandwidthEstimator::latency() const {
    return rls_.b(0) / 2.f;
}

float BandwidthEstimator::bandwidth() const {
    return 1 / rls_.b(1);
}

float BandwidthEstimator::header_size() const {
    return header_size_;
}

float BandwidthEstimator::dt(float payload_size) const {
    return (payload_size + header_size()) / bandwidth();
}

void BandwidthEstimator::print(std::ostream& ostr) const {
    ostr << rls_ << " T: " << latency() << ", B: " << bandwidth() << ", H: " << header_size();
}

std::ostream& Mlib::operator << (std::ostream& ostr, const BandwidthEstimator& be) {
    be.print(ostr);
    return ostr;
}
