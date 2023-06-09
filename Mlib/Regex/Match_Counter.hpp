#pragma once
#include <cstddef>

#define BEGIN_MATCH_COUNTER static size_t counter = 1
#define DECLARE_MATCH_COUNTER(a) static const size_t a = counter++;
