#pragma once
#include <Mlib/Render/IContext.hpp>

struct ANativeWindow;

class AContext: public Mlib::IContext {
public:
    AContext();
    ~AContext();
    bool is_initialized() const override;
};
