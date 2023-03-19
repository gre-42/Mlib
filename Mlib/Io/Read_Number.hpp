#pragma once
#include <iosfwd>

namespace Mlib {

template <class TNum>
std::istream& read_num(std::istream& istr, TNum& num);

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
