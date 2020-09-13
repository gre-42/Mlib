#pragma once

namespace Mlib {

const char* getenv_default(const char* name, const char* deflt);

float getenv_default_float(const char* n, float deflt);

int getenv_default_int(const char* n, int deflt);

bool getenv_default_bool(const char* n, bool deflt);

}
