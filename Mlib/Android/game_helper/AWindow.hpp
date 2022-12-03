#pragma once
#include <Mlib/Render/IWindow.hpp>

struct ANativeWindow;

class AWindow: public Mlib::IWindow {
public:
    explicit AWindow(ANativeWindow& native_window);
    ~AWindow();
    void make_current() const override;
    void unmake_current() const override;
    bool is_initialized() const override;
private:
    ANativeWindow& native_window_;
};
