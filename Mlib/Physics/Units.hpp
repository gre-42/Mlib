#pragma once
#include <cmath>

namespace Mlib {

static const float milli = 1.f / 1000.f;
static const float centi = 1.f / 100.f;
static const float percent = 1.f / 100.f;
static const float kilo = 1000.f;

static const float seconds = float{1e3};
static const float meters = 1.f;
static const float kg = float(1e-3);

static const float radians = 1.f;
static const float degrees = float(M_PI / 180);

static const float minutes = 60.f * seconds;
static const float hours = 60.f * minutes;

static const float mm = milli * meters;
static const float cm = centi * meters;
static const float km = kilo * meters;

static const float kph = km / hours;
static const float N = kg * meters / (seconds * seconds);
static const float W = N * meters / seconds;
static const float hp = 735.5f * W;
static const float rpm = float(2 * M_PI) / minutes;
static const float rps = float(2 * M_PI) / seconds;
static const float J = N * meters;

static const float Hz = 1 / seconds;

}
