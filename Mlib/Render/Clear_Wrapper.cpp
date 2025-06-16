#include "Clear_Wrapper.hpp"
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Instance_Handles/Render_Guards.hpp>
#include <Mlib/Render/Render_Logics/Clear_Logic.hpp>

using namespace Mlib;

static std::unique_ptr<ClearLogic> clear_logic;

#ifdef __ANDROID__
void Mlib::clear_color(const FixedArray<float, 4>& color, ClearBackend backend) {
    if (clear_logic == nullptr) {
        THROW_OR_ABORT("Clear logic not set");
    }
    clear_logic->clear_color(color);
}
void Mlib::clear_depth(ClearBackend backend) {
    if (clear_logic == nullptr) {
        THROW_OR_ABORT("Clear logic not set");
    }
    clear_logic->clear_depth();
}
void Mlib::clear_color_and_depth(const FixedArray<float, 4>& color, ClearBackend backend) {
    if (clear_logic == nullptr) {
        THROW_OR_ABORT("Clear logic not set");
    }
    clear_logic->clear_color_and_depth(color);
}
#else
void Mlib::clear_color(const FixedArray<float, 4>& color, ClearBackend backend) {
    if (backend == ClearBackend::SHADER) {
        if (clear_logic == nullptr) {
            THROW_OR_ABORT("Clear logic not set");
        }
        clear_logic->clear_color(color);
    } else {
        notify_rendering(CURRENT_SOURCE_LOCATION);
        CHK(glClearColor(color(0), color(1), color(2), color(3)));
        CHK(glClear(GL_COLOR_BUFFER_BIT));
    }
}
void Mlib::clear_depth(ClearBackend backend) {
    if (backend == ClearBackend::SHADER) {
        if (clear_logic == nullptr) {
            THROW_OR_ABORT("Clear logic not set");
        }
        clear_logic->clear_depth();
    } else {
        notify_rendering(CURRENT_SOURCE_LOCATION);
        CHK(glClear(GL_DEPTH_BUFFER_BIT));
    }
}
void Mlib::clear_color_and_depth(const FixedArray<float, 4>& color, ClearBackend backend) {
    if (backend == ClearBackend::SHADER) {
        if (clear_logic == nullptr) {
            THROW_OR_ABORT("Clear logic not set");
        }
        clear_logic->clear_color_and_depth(color);
    } else {
        notify_rendering(CURRENT_SOURCE_LOCATION);
        CHK(glClearColor(color(0), color(1), color(2), color(3)));
        CHK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    }
}
#endif

ClearWrapperGuard::ClearWrapperGuard() {
    if (clear_logic != nullptr) {
        THROW_OR_ABORT("Clear logic already set");
    }
    clear_logic = std::make_unique<ClearLogic>();
}

ClearWrapperGuard::~ClearWrapperGuard() {
    if (clear_logic == nullptr) {
        verbose_abort("Clear logic not set");
    }
    clear_logic.reset();
}
