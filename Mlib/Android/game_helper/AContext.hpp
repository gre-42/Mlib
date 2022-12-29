#pragma once
#include <Mlib/Render/IContext.hpp>

struct ALooper;

class AContext: public Mlib::IContext {
public:
    AContext();
    ~AContext();
    bool is_initialized() const override;
private:
    ALooper* main_thread_looper_;
};
