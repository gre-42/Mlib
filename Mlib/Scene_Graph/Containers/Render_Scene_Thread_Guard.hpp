#pragma once

namespace Mlib {

class Scene;

class RenderSceneThreadGuard {
public:
    explicit RenderSceneThreadGuard(const Scene& scene);
    ~RenderSceneThreadGuard();
private:
    Scene& scene_;
};

}
