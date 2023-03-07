#pragma once

/*
 * zlib License
 *
 * Regularized Incomplete Beta Function
 *
 * Copyright (c) 2016, 2017 Lewis Van Winkle
 * http://CodePlea.com
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgement in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */


#include <Mlib/Throw_Or_Abort.hpp>
#include <math.h>

#define STOP 1.0e-8
#define TINY 1.0e-30

template <class TData>
TData incbeta(
    TData a,
    TData b,
    TData x,
    TData stop = (TData)1e-8,
    TData tiny = (TData)1e-30)
{
    if (x < (TData)0 || x > (TData)1) {
        THROW_OR_ABORT("Invalid input for incbeta");
    }

    /*The continued fraction converges nicely for x < (a+1)/(a+b+2)*/
    if (x > (a + (TData)1) / (a + b + (TData)2)) {
        return ((TData)1 - incbeta<TData>(b, a, (TData)1 - x)); /*Use the fact that beta is symmetrical.*/
    }

    /*Find the first part before the continued fraction.*/
    const TData lbeta_ab = lgamma(a) + lgamma(b) - lgamma(a+b);
    const TData front = exp(log(x) * a + log((TData)1 - x) * b - lbeta_ab) / a;

    /*Use Lentz's algorithm to evaluate the continued fraction.*/
    TData f = (TData)1, c = (TData)1, d = (TData)0;

    for (int i = 0; i <= 200; ++i) {
        int m = i / 2;

        TData numerator;
        if (i == 0) {
            numerator = (TData)1; /*First numerator is 1.0.*/
        } else if (i % 2 == 0) {
            numerator = ((TData)m * (b - (TData)m) * x)/((a + TData(2 * m - 1))*(a + TData(2 * m))); /*Even term.*/
        } else {
            numerator = -((a + (TData)m) * (a + b + (TData)m) * x) / ((a + TData(2 * m)) * (a + TData(2 * m + 1))); /*Odd term.*/
        }

        /*Do an iteration of Lentz's algorithm.*/
        d = (TData)1 + numerator * d;
        if (std::abs(d) < tiny) d = tiny;
        d = (TData)1 / d;

        c = (TData)1 + numerator / c;
        if (fabs(c) < tiny) c = tiny;

        const TData cd = c * d;
        f *= cd;

        /*Check for stop.*/
        if (std::abs((TData)1 - cd) < stop) {
            return front * (f - (TData)1);
        }
    }

    THROW_OR_ABORT("incbeta did not converge"); /*Needed more loops, did not converge.*/
}
