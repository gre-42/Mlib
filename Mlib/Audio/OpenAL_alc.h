#pragma once

// Taken from AL/alut.h
#if defined(_MSC_VER)
#include <alc.h>
#elif defined(__APPLE__)
#include <OpenAL/alc.h>
#else
#include <AL/alc.h>
#endif
