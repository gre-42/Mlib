#pragma once

namespace Mlib {

// From: https://github.com/sczesla/PyAstronomy/blob/master/src/pyasl/asl/eq2hor.py
void hadec2altaz(double ha, double dec, double lat, double& alt, double& az);

}
