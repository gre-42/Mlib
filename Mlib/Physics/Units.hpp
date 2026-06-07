#pragma once
#include <cmath>

namespace Mlib {

static constexpr const float milli = 1.f / 1000.f;
static constexpr const float centi = 1.f / 100.f;
static constexpr const float percent = 1.f / 100.f;
static constexpr const float kilo = 1000.f;

static constexpr const float seconds = float{1e3};
static constexpr const float meters = 1.f;
static constexpr const float kg = float(1e-3);

static constexpr const float radians = 1.f;
static constexpr const float degrees = float(M_PI / 180);

static constexpr const float minutes = 60.f * seconds;
static constexpr const float hours = 60.f * minutes;

static constexpr const float mm = milli * meters;
static constexpr const float cm = centi * meters;
static constexpr const float km = kilo * meters;

static constexpr const float kph = km / hours;
static constexpr const float N = kg * meters / (seconds * seconds);
static constexpr const float W = N * meters / seconds;
static constexpr const float hp = 735.5f * W;
static constexpr const float rpm = float(2 * M_PI) / minutes;
static constexpr const float rps = float(2 * M_PI) / seconds;
static constexpr const float J = N * meters;

static constexpr const float Hz = 1 / seconds;

}
