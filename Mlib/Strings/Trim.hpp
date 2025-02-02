#pragma once
#include <string>

// From: https://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring

namespace Mlib {

// Trim spaces

// Trim from start (in place)
void ltrim(std::string &s);

// Trim from end (in place)
void rtrim(std::string &s);

// Trim from both ends (in place)
void trim(std::string &s);

// Trim from start (copying)
std::string ltrim_copy(std::string s);

// Trim from end (copying)
std::string rtrim_copy(std::string s);

// Trim from both ends (copying)
std::string trim_copy(std::string s);

// Trim character

// Trim from start (in place)
void ltrim(std::string &s, char c);

// Trim from end (in place)
void rtrim(std::string &s, char c);

// Trim from both ends (in place)
void trim(std::string &s, char c);

// Trim from start (copying)
std::string ltrim_copy(std::string s, char c);

// Trim from end (copying)
std::string rtrim_copy(std::string s, char c);

// Trim from both ends (copying)
std::string trim_copy(std::string s, char c);

}
