#pragma once
#include <compare>
#include <list>
#include <string>
#include <vector>

namespace Mlib {

double safe_stod(const std::string& s);
float safe_stof(const std::string& s);
int safe_stoi(const std::string& s);
bool safe_stob(const std::string& s);
std::strong_ordering operator <=> (const std::string& a, const std::string& b);
std::list<std::string> string_to_list(const std::string& str);
std::vector<std::string> string_to_vector(const std::string& str);

}
