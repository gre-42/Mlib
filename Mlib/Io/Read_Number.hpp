#pragma once
#include <Mlib/Strings/To_Number.hpp>
#include <cmath>
#include <cstdint>
#include <iosfwd>
#include <istream>
#include <string>
#include <type_traits>

namespace Mlib {

template <class TNum>
std::istream& read_num(std::istream& istr, TNum& num) {
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

template <class TNum>
class ReadNum {
    template <class TNum2>
    friend std::istream& operator >> (std::istream& istr, const ReadNum<TNum2>& num);
public:
    ReadNum(TNum& num)
    : num_{num}
    {}
private:
    TNum& num_;
};

template <class TNum>
std::istream& operator >> (std::istream& istr, const ReadNum<TNum>& num) {
    return read_num(istr, num.num_);
}

}
