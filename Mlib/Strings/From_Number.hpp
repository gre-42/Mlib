#pragma once
#include <string>

namespace Mlib {

double safe_stod(const std::string& s);
float safe_stof(const std::string& s);
int safe_stoi(const std::string& s);
unsigned int safe_stou(const std::string& s);
size_t safe_stoz(const std::string& s);
bool safe_stob(const std::string& s);

}
