#include "Clear_Wrapper.hpp"
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Render_Logics/Clear_Logic.hpp>

using namespace Mlib;

ClearLogic& clear_logic() {
    static ClearLogic cl;
    return cl;
}

//#ifdef __ANDROID__
void Mlib::clear_color(const FixedArray<float, 4>& color) {
    clear_logic().clear_color(color);
}
void Mlib::clear_depth() {
    clear_logic().clear_depth();
}
void Mlib::clear_color_and_depth(const FixedArray<float, 4>& color) {
    clear_logic().clear_color_and_depth(color);
}
//#else
//void Mlib::clear_color(const FixedArray<float, 4>& color) {
//    CHK(glClearColor(color(0), color(1), color(2), color(3)));
//    CHK(glClear(GL_COLOR_BUFFER_BIT));
//}
//void Mlib::clear_depth() {
//    CHK(glClear(GL_DEPTH_BUFFER_BIT));
//}
//void Mlib::clear_color_and_depth(const FixedArray<float, 4>& color) {
//    CHK(glClearColor(color(0), color(1), color(2), color(3)));
//    CHK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
//}
//#endif
