#include "AUi.hpp"
#include <emscripten/console.h>

using namespace Mlib;

void AUi::ShowMessage(
    const std::string& title,
    const std::string& message)
{
    emscripten_console_log((title + ": " + message).c_str());
}

std::string AUi::GetFlavor() {
    return "extended";
}
