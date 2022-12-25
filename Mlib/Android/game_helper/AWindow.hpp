#pragma once
#include <Mlib/Render/IWindow.hpp>

struct android_app;

class AWindow: public Mlib::IWindow {
public:
    explicit AWindow(android_app& app);
    ~AWindow();
    void make_current() const override;
    void unmake_current() const override;
    void set_frame_rate_if_supported(float rate) const;
private:
    android_app& app_;
};
