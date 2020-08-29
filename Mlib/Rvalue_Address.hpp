#pragma once

namespace Mlib {

/**
 * Get rid of warning "taking address of a temporary".
 * Source: https://stackoverflow.com/a/9963084/2292832
 */
template<typename T> const T* rvalue_address(const T& in) {
    return &in;
}

}
