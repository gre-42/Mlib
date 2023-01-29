#pragma once
#include <atomic>
#include <list>
#include <optional>
#include <tuple>

namespace Mlib {

#ifdef __ANDROID__
struct Viewport {
    int x;
    int y;
    int width;
    int height;
};
#else
struct Viewport {
    float x;
    float y;
    float width;
    float height;
};
#endif

class IPixelRegion;

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
    static std::optional<ViewportGuard> from_widget(
        const IPixelRegion &ew);
    static std::optional<ViewportGuard> periodic(
        float x,
        float y,
        float width,
        float height,
        int screen_width,
        int screen_height);
    ~ViewportGuard();
    float fwidth() const;
    float fheight() const;
    int iwidth() const;
    int iheight() const;
private:
    void apply() const;
    Viewport viewport_;
    ViewportGuard* prev_guard_;
    static std::atomic<ViewportGuard*> current_guard_;
};

}
