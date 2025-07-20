#pragma once
#include <cstdint>

#define BEGIN_MATCH_COUNTER static uint32_t counter = 1
#define DECLARE_MATCH_COUNTER(a) static const uint32_t a = counter++;
