#include "Write_Number.hpp"
#include <cmath>
#include <cstdint>
#include <ostream>
#include <string>
#include <type_traits>

using namespace Mlib;

template <class TNum>
std::ostream& Mlib::write_num(std::ostream& ostr, const TNum& num) {
    if constexpr (std::is_floating_point_v<TNum>) {
        if (std::isnan(num)) {
            return (ostr << "nan");
        }
        if (num == INFINITY) {
            return (ostr << "inf");
        }
        if (num == -INFINITY) {
            return (ostr << "-inf");
        }
    }
    return (ostr << num);
}

template std::ostream& Mlib::write_num<double>(std::ostream& ostr, const double& num);
template std::ostream& Mlib::write_num<float>(std::ostream& ostr, const float& num);
template std::ostream& Mlib::write_num<int32_t>(std::ostream& ostr, const int32_t& num);
template std::ostream& Mlib::write_num<uint32_t>(std::ostream& ostr, const uint32_t& num);
template std::ostream& Mlib::write_num<int64_t>(std::ostream& ostr, const int64_t& num);
template std::ostream& Mlib::write_num<uint64_t>(std::ostream& ostr, const uint64_t& num);
template std::ostream& Mlib::write_num<std::string>(std::ostream& ostr, const std::string& num);
