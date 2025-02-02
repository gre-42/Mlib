#pragma once
#include <Mlib/Render/IContext.hpp>
#include <thread>

class AContext: public Mlib::IContext {
public:
    AContext();
    ~AContext();
    bool is_initialized() const override;
private:
    std::thread::id main_thread_id_;
};
