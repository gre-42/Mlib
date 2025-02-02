#pragma once

// Taken from AL/alut.h
#if defined(_MSC_VER)
#include <al.h>
#elif defined(__APPLE__)
#include <OpenAL/al.h>
#else
#include <AL/al.h>
#endif
