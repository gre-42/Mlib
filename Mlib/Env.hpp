#pragma once
#include <cstddef>
#include <optional>
#include <string>

namespace Mlib {

std::optional<std::string> try_getenv(const char* name);
std::string str_getenv(const char * name);
const char* getenv_default(const char* name, const char* deflt);
float getenv_default_float(const char* n, float deflt);
int getenv_default_int(const char* n, int deflt);
unsigned int getenv_default_uint(const char* n, unsigned int deflt);
size_t getenv_default_size_t(const char* n, size_t deflt);
bool getenv_default_bool(const char* n, bool deflt);

#ifndef __ANDROID__
void set_app_reldir(const std::string& app_reldir);
#endif

std::string get_appdata_directory();
std::string get_path_in_appdata_directory(const std::initializer_list<std::string>& child_path);

}
