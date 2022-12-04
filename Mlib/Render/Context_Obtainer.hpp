#pragma once

namespace Mlib {

class IWindow;

class ContextObtainer {
public:
    static bool is_initialized();
    static void set_window(IWindow& window);
private:
    static IWindow& window();
    static IWindow* window_;
};

}
