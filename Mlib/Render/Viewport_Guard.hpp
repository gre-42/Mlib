#pragma once
#include <atomic>
#include <list>
#include <optional>
#include <tuple>

namespace Mlib {

class ViewportGuard {
    ViewportGuard(const ViewportGuard&) = delete;
    ViewportGuard& operator = (const ViewportGuard&) = delete;
public:
    ViewportGuard(
        float x,
        float y,
        float width,
        float height);
    ViewportGuard(
        int width,
        int height);
    static std::optional<ViewportGuard> periodic(
        float x,
        float y,
        float width,
        float height,
        int screen_width,
        int screen_height);
    ~ViewportGuard();
    const float width;
    const float height;
private:
    std::tuple<float, float, float, float> viewport_;
    ViewportGuard* prev_guard_;
    static std::atomic<ViewportGuard*> current_guard_;
};

}
