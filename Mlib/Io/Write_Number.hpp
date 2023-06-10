#pragma once
#include <cmath>
#include <cstdint>
#include <iosfwd>
#include <ostream>
#include <string>
#include <type_traits>

namespace Mlib {

template <class TNum>
std::ostream& write_num(std::ostream& ostr, const TNum& num) {
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

template <class TNum>
class WriteNum {
    template <class TNum2>
    friend std::ostream& operator << (std::ostream& ostr, const WriteNum<TNum2>& num);
public:
    WriteNum(const TNum& num)
    : num_{num}
    {}
private:
    const TNum& num_;
};

template <class TNum>
std::ostream& operator << (std::ostream& ostr, const WriteNum<TNum>& num) {
    return write_num(ostr, num.num_);
}

}
