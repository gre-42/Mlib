#pragma once
#include <Mlib/Strings/Utf8_Path.hpp>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>

namespace Mlib {

class AUi {
public:
    static void ShowMessage(
        const std::string& title,
        const std::string& message);
    static std::string GetFlavor();
};

}
