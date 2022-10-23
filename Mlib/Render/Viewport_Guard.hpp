#pragma once
#include <list>
#include <tuple>

namespace Mlib {

enum class Periodicity {
    PERIODIC,
    APERIODIC
};

class ViewportGuard {
public:
    ViewportGuard(
        float x,
        float y,
        float width,
        float height);
    ViewportGuard(
        int width,
        int height);
    ViewportGuard(
        float x,
        float y,
        float width,
        float height,
        int screen_width,
        int screen_height,
        Periodicity position_periodicity);
    ~ViewportGuard();
private:
    static std::list<std::tuple<float, float, float, float>> stack_;
};

}
