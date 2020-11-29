#pragma once

namespace Mlib {

template <class TData, class TFunc>
TData find_right_boundary_of_maximum(const TFunc& f, const TData& left_boundary, const TData& step_size) {
    TData x = left_boundary;
    TData f_old = f(x);
    while(true) {
        x += 0.01;
        TData f_x = f(x);
        if (f_x < f_old) {
            break;
        }
        f_old = f_x;
    }
    return x;
}

}
