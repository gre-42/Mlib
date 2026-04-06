#pragma once

namespace Mlib {

// From: https://stackoverflow.com/a/12940576/2292832
template <class TBegin0, class TEnd0, class TBegin1, class TEnd1>
bool empty_intersection(
    const TBegin0& begin0, const TEnd0& end0,
    const TBegin1& begin1, const TEnd1& end1)
{
    auto i0 = begin0;
    auto i1 = begin1;
    while (i0 != end0 && i1 != end1)
    {
        if (*i0 < *i1)
            ++i0;
        else if (*i1 < *i0)
            ++i0;
        else
            return false;
    }
    return true;
}

}
