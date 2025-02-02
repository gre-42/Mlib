#include "Local_Mean_Sidereal_Time.hpp"
#include <Mlib/Geography/Idl_Mod.hpp>
#include <Mlib/Math/Math.hpp>
#include <Mlib/Physics/Units.hpp>

using namespace Mlib;

double Mlib::ct2lst(double jddays, double lon) {
    // Useful constants, see Meeus, p.84
    const double c[] = {
        280.46061837, 360.98564736629,
        0.000387933, 38710000.0};
    auto time = jddays / 36525.0;

    // Compute GST in seconds.
    auto theta = (c[0] + (c[1] * jddays) + (c[2] - time / c[3]) * squared(time)) * degrees;

    // Compute LST in hours.
    auto lst = (theta + lon) / (15.0 * degrees);
    if (lst < 0.0) {
        lst = 24.0 + idl_mod(lst, 24.);
    }

    // Local sidereal time in hours (0. to 24.)
    lst = idl_mod(lst, 24.0);

    return lst;
}
