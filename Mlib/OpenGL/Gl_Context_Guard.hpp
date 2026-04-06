#pragma once

namespace Mlib {

class IWindow;

class GlContextGuard {
    GlContextGuard(const GlContextGuard&) = delete;
    GlContextGuard& operator = (const GlContextGuard&) = delete;
public:
    explicit GlContextGuard(const IWindow& window);
    ~GlContextGuard();
    static void assert_this_thread_is_renderer();
private:
    const IWindow& window_;
};

}
