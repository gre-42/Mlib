#include "Trim.hpp"
#include <algorithm> 
#include <cctype>
#include <locale>

using namespace Mlib;

// From: https://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring

// Trim spaces

void Mlib::ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}

void Mlib::rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

void Mlib::trim(std::string &s) {
    ltrim(s);
    rtrim(s);
}

std::string Mlib::ltrim_copy(std::string s) {
    ltrim(s);
    return s;
}

std::string Mlib::rtrim_copy(std::string s) {
    rtrim(s);
    return s;
}

std::string Mlib::trim_copy(std::string s) {
    trim(s);
    return s;
}

// Trim spaces

void Mlib::ltrim(std::string &s, char c) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [c](unsigned char ch) {
        return ch != c;
    }));
}

void Mlib::rtrim(std::string &s, char c) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [c](unsigned char ch) {
        return ch != c;
    }).base(), s.end());
}

void Mlib::trim(std::string &s, char c) {
    ltrim(s, c);
    rtrim(s, c);
}

std::string Mlib::ltrim_copy(std::string s, char c) {
    ltrim(s, c);
    return s;
}

std::string Mlib::rtrim_copy(std::string s, char c) {
    rtrim(s, c);
    return s;
}

std::string Mlib::trim_copy(std::string s, char c) {
    trim(s, c);
    return s;
}
