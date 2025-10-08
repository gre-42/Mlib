#pragma once

#define AL_ALEXT_PROTOTYPES

// Taken from AL/alut.h
#if defined(_MSC_VER)
#include <efx.h>
#elif defined(__APPLE__)
#include <OpenAL/efx.h>
#else
#include <AL/efx.h>
#endif
