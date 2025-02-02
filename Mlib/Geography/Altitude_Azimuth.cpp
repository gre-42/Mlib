#include "Altitude_Azimuth.hpp"
#include <Mlib/Math/Math.hpp>
#include <Mlib/Physics/Units.hpp>

using namespace Mlib;

void Mlib::hadec2altaz(double ha, double dec, double lat, double& alt, double& az) {
    auto sh = std::sin(ha);
    auto ch = std::cos(ha);
    auto sd = std::sin(dec);
    auto cd = std::cos(dec);
    auto sl = std::sin(lat);
    auto cl = std::cos(lat);

    auto x = - ch * cd * sl + sd * cl;
    auto y = - sh * cd;
    auto z = ch * cd * cl + sd * sl;
    auto r = std::sqrt(squared(x) + squared(y));

    // Now get Alt, Az
    az = std::atan2(y, x);
    alt = std::atan2(z, r);

    // Correct for negative AZ
    if (az < 0) {
        az += 360. * degrees;
    }
}
