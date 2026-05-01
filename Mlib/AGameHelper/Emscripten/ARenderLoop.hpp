#pragma once
#include <functional>

namespace Mlib {

class AEngine;

class ARenderLoop {
public:
    explicit ARenderLoop(AEngine& aengine);
    ~ARenderLoop();

    void render_loop(const std::function<bool()>& exit_loop = [](){return false;});
    bool destroy_requested() const;
private:
    AEngine& aengine_;
};

}
