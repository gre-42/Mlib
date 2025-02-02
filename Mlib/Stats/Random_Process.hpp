#pragma once

namespace Mlib {

template <class TRng, class TSmoother>
class RandomProcess {
public:
    RandomProcess(
        const TRng& rng,
        const TSmoother& smoother)
    : rng_{ rng },
      smoother_{ smoother }
    {}

    auto operator () () {
        return smoother_(rng_());
    }
private:
    TRng rng_;
    TSmoother smoother_;
};

}
