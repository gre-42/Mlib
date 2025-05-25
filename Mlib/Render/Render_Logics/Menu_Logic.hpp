#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Render/Fullscreen_Callback.hpp>
#include <Mlib/Render/Ui/Button_Press.hpp>

namespace Mlib {

class ButtonStates;
class UiFocuses;
class MenuLogicKeys;

class MenuUserClass {
public:
    ButtonStates& button_states;
    UiFocuses& ui_focuses;
};

class MenuLogic {
public:
    explicit MenuLogic(MenuUserClass& user_object);
    ~MenuLogic();

    void handle_events();

private:
    MenuUserClass& user_object_;
    std::unique_ptr<MenuLogicKeys> keys_;
};

}
