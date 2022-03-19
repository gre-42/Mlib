#pragma once
#include <cmath>

namespace Mlib {

static const float s = float{1e3};
static const float meters = 1.f;
static const float Kg = 1.f;

static const float radians = 1.f;
static const float degrees = float(M_PI / 180);

static const float minutes = 60.f * s;
static const float hours = 60.f * minutes;

static const float kilo = 1000.f;
static const float kph = kilo * meters / hours;
static const float N = Kg * meters / (s * s);
static const float W = N * meters / s;
static const float rpm = float(2 * M_PI) / minutes;

}
