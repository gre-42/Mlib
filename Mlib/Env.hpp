#pragma once
#include <cstddef>
#include <string>

namespace Mlib {

const char* getenv_default(const char* name, const char* deflt);

float getenv_default_float(const char* n, float deflt);

int getenv_default_int(const char* n, int deflt);

size_t getenv_default_size_t(const char* n, size_t deflt);

bool getenv_default_bool(const char* n, bool deflt);

std::string get_home_directory();

std::string get_path_in_home_directory(const std::string& child_path);

}
