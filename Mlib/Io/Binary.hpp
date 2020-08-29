#pragma once
#include <string>

namespace Mlib {

template <class TData>
void write_binary(std::ostream& ostr, const TData& v) {
    ostr.write((char*)&v, sizeof(TData));
}

template <class TData>
void read_binary(std::istream& istr, TData& v) {
    istr.read((char*)&v, sizeof(TData));
}


}
