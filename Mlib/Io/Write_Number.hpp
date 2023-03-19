#pragma once
#include <iosfwd>

namespace Mlib {

template <class TNum>
std::ostream& write_num(std::ostream& ostr, const TNum& num);

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
