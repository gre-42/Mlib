#include "Read_Number.hpp"
#include <Mlib/Strings/To_Number.hpp>
#include <cmath>
#include <cstdint>
#include <istream>
#include <string>
#include <type_traits>

using namespace Mlib;

template <class TNum>
std::istream& Mlib::read_num(std::istream& istr, TNum& num) {
    if constexpr (std::is_floating_point_v<TNum>) {
        std::string str;
        istr >> str;
        if (!istr.fail() || (istr.eof() && !str.empty())) {
            num = safe_sto<TNum>(str);
        }
        return istr;
    } else {
        return (istr >> num);
    }
}

template std::istream& Mlib::read_num<double>(std::istream& istr, double& num);
template std::istream& Mlib::read_num<float>(std::istream& istr, float& num);
template std::istream& Mlib::read_num<int32_t>(std::istream& istr, int32_t& num);
template std::istream& Mlib::read_num<uint32_t>(std::istream& istr, uint32_t& num);
template std::istream& Mlib::read_num<int64_t>(std::istream& istr, int64_t& num);
template std::istream& Mlib::read_num<uint64_t>(std::istream& istr, uint64_t& num);
template std::istream& Mlib::read_num<std::string>(std::istream& istr, std::string& num);
