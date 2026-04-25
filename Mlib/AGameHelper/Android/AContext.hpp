#pragma once
#include <Mlib/OpenGL/IContext.hpp>
#include <thread>

namespace Mlib {

class AContext: public IContext {
public:
    AContext();
    ~AContext();
    bool is_initialized() const override;
private:
    std::thread::id main_thread_id_;
};

}
