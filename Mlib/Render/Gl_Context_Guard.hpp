#pragma once

namespace Mlib {

class IWindow;

class GlContextGuard {
    GlContextGuard(const GlContextGuard&) = delete;
    GlContextGuard& operator = (const GlContextGuard&) = delete;
public:
    explicit GlContextGuard(const IWindow& window);
    ~GlContextGuard();
private:
    const IWindow& window_;
};

}
