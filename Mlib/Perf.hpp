#pragma once
#include <chrono>
#include <iomanip>
#include <iostream>
#include <string>

namespace Mlib {

void perf(const std::string& name) {
    static std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cerr << name << " " << std::setw(10) << std::right << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << std::endl;
    begin = end;
}

}
