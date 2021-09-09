#pragma once
#include <Mlib/Array/Array.hpp>
#include <Mlib/Images/Total_Variation/Huber_Rof_Config.hpp>

namespace Mlib::HuberRof {

enum class Regularizer {
    FORWARD_BACKWARD_DIFFERENCES,
    FORWARD_BACKWARD_WEIGHTING,
    CENTRAL_DIFFERENCES,
    DIFFERENCE_OF_BOXES  // a.k.a. Laplace
};

static const Regularizer regularizer = Regularizer::FORWARD_BACKWARD_WEIGHTING;

struct EdgeImageConfig;

float xsum(const Array<float>& v);

Array<float> sum_q(const Array<float>& q);

Array<float> AGd(
    const Array<float>& g,
    const Array<float>& d);

Array<float> delta(const Array<float>& q);

Array<float> energy_primal(
    const Array<float>& g,
    float theta,
    float epsilon,
    const Array<float>& d,
    const Array<float>& a);

Array<float> energy_dual(
    const Array<float>& g,
    float theta,
    float epsilon,
    const Array<float>& d,
    const Array<float>& a,
    const Array<float>& q);

Array<float> energy_dq(
    const Array<float>& g,
    float epsilon,
    const Array<float>& d,
    const Array<float>& q);

Array<float> energy_dd(
    const Array<float>& g,
    float theta,
    const Array<float>& d,
    const Array<float>& a,
    const Array<float>& q);

float prox_sigma_fs(
    float sigma,
    const Array<float>& g,
    float epsilon,
    const Array<float>& dm,
    const Array<float>& q);

Array<float> prox_sigma_fs_dq(
    float sigma,
    const Array<float>& g,
    float epsilon,
    const Array<float>& dm,
    const Array<float>& q);

float prox_tau_gs(
    float tau,
    const Array<float>& g,
    float theta,
    const Array<float>& d,
    const Array<float>& a,
    const Array<float>& q,
    bool zero_sum = false);

Array<float> prox_tau_gs_dd(
    float tau,
    const Array<float>& g,
    float theta,
    const Array<float>& d,
    const Array<float>& a,
    const Array<float>& q);

Array<float> g_from_grayscale(
    const Array<float>& im_ref_gray,
    const EdgeImageConfig& edge_image_config);

class HuberRofSolver {
public:
    explicit HuberRofSolver(
        const Array<float>& g,
        bool print_debug = false,
        bool print_bmps = false);
    void initialize_q();
    void iterate(const HuberRof::HuberRofConfig& config);
    Array<float> d_;
    Array<float> a_;
    Array<float> q_;
    Array<float> g_;
    size_t n_;
private:
    bool print_debug_;
    bool print_bmps_;
};

}
