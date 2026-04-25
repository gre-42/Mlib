#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <numbers>
#include <span>

namespace Mlib {

class BiquadFilter {
public:
    BiquadFilter(const std::array<float, 3>& b, const std::array<float, 2>& a)
        : b_{b}
        , a_{a}
    {}

    static BiquadFilter low_pass(float cutoff_freq, float sample_rate, float Q = 0.707f) {
        float omega = 2 * std::numbers::pi_v<float> * cutoff_freq / sample_rate;
        float sn = std::sin(omega);
        float cs = std::cos(omega);
        float alpha = sn / (2 * Q);

        float a0 = 1 + alpha;
        return BiquadFilter{
            {
                ((1 - cs) / 2) / a0,
                (1 - cs) / a0,
                ((1 - cs) / 2) / a0
            },
            {
                (-2 * cs) / a0,
                (1 - alpha) / a0
            }
        };
    }

    static BiquadFilter low_shelf(float freq, float sample_rate, float gain_lp, float shelf_slope = 1) {
        float A = std::sqrt(gain_lp);
        float omega = 2 * std::numbers::pi_v<float> * freq / sample_rate;
        float sn = std::sin(omega);
        float cs = std::cos(omega);
        float alpha = sn / 2 * std::sqrt((A + 1 / A) * (1 / shelf_slope - 1) + 2);
        float twoRootAalpha = 2 * std::sqrt(A) * alpha;

        float a0 = (A + 1) + (A - 1) * cs + twoRootAalpha;
        return BiquadFilter{
            {
                A * ((A + 1) - (A - 1) * cs + twoRootAalpha) / a0,
                2 * A * ((A - 1) - (A + 1) * cs) / a0,
                A * ((A + 1) - (A - 1) * cs - twoRootAalpha) / a0
            },
            {
                -2 * ((A - 1) + (A + 1) * cs) / a0,
                ((A + 1) + (A - 1) * cs - twoRootAalpha) / a0
            }
        };
    }

    template <typename T>
    void process(std::span<T> samples, float gain = 1.f) {
        for (auto& sample : samples) {
            float in = gain * static_cast<float>(sample);
            float out = b_[0]*in + b_[1]*z_[0] + b_[2]*z_[1] - a_[0]*y_[0] - a_[1]*y_[1];

            z_[1] = z_[0]; z_[0] = in;
            y_[1] = y_[0]; y_[0] = out;

            sample = static_cast<T>(std::clamp(
                out,
                (float)std::numeric_limits<T>::lowest(),
                (float)std::numeric_limits<T>::max()));
        }
    }

    template <typename T>
    static void process(
        std::span<T> samples,
        float freq,
        float sample_rate,
        float gain_lp,
        float gain_hp)
    {
        low_shelf(freq, sample_rate, gain_lp / gain_hp).process(samples, gain_hp);
    }

    void reset() {
        z_[0] = z_[1] = y_[0] = y_[1] = 0;
    }

private:
    std::array<float, 3> b_;
    std::array<float, 2> a_;
    std::array<float, 2> z_ = {0, 0};
    std::array<float, 2> y_ = {0, 0};
};

}
