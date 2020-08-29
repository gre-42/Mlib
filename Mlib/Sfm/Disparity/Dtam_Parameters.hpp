#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <cstddef>
#include <iosfwd>

namespace Mlib { namespace Sfm { namespace Dm {

class DtamParameters {
public:
    DtamParameters();
    DtamParameters(
        float min_depth,
        float max_depth,
        size_t ndepths,
        float alpha_G,
        float beta_G,
        float theta_0,
        float theta_end,
        float beta,
        float lambda,
        float epsilon,
        size_t nsteps);
    float min_depth__;
    float max_depth__;
    size_t ndepths__;
    float alpha_G_;
    float beta_G_;
    float theta_0__;
    float theta_end__;
    float beta_;
    float lambda_;
    float epsilon_;
    size_t nsteps_;
    Array<float> inverse_depths() const;
    float theta_0_corrected() const;
    float theta_end_corrected() const;
private:
    float theta_correction_factor() const;
};

std::ostream& operator << (std::ostream& str, const DtamParameters& params);

}}}
