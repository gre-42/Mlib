#pragma once

namespace Mlib {

template <class TData, class TFunc>
TData find_maximum_right_boundary(const TFunc& f, const TData& left_boundary, const TData& step_size) {
    TData x = 0;
    TData f_old = f(0);
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
