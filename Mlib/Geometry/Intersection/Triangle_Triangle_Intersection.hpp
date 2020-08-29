#pragma once
#include <Mlib/Array/Fixed_Array.hpp>

/* From:
    https://github.com/benardp/contours/blob/master/freestyle/view_map/triangle_triangle_intersection.c
    https://gamedev.stackexchange.com/questions/88060/triangle-triangle-intersection-code
*/

namespace tti {

int tri_tri_overlap_test_3d(const float p1[3], const float q1[3], const float r1[3],
                            const float p2[3], const float q2[3], const float r2[3]);


int coplanar_tri_tri3d(const float  p1[3], const float  q1[3], const float  r1[3],
                       const float  p2[3], const float  q2[3], const float  r2[3],
                       const float  N1[3], const float  N2[3]);


int tri_tri_overlap_test_2d(const float p1[2], const float q1[2], const float r1[2],
                            const float p2[2], const float q2[2], const float r2[2]);


int tri_tri_intersection_test_3d(const float p1[3], const float q1[3], const float r1[3],
                                 const float p2[3], const float q2[3], const float r2[3],
                                 int * coplanar,
                                 float source[3], float target[3]);

}

namespace Mlib {

inline bool tri_tri_intersection_test_3d(
    const FixedArray<FixedArray<float, 3>, 3>& tri0,
    const FixedArray<FixedArray<float, 3>, 3>& tri1,
    bool& coplanar,
    FixedArray<FixedArray<float, 3>, 2>& intersection_line)
{
    int icoplanar = 0;
    bool res = ::tti::tri_tri_intersection_test_3d(
        tri0(0).flat_begin(),
        tri0(1).flat_begin(),
        tri0(2).flat_begin(),
        tri1(0).flat_begin(),
        tri1(1).flat_begin(),
        tri1(2).flat_begin(),
        &icoplanar,
        intersection_line(0).flat_begin(),
        intersection_line(1).flat_begin()) == 1;
    coplanar = icoplanar;
    return res;
}

}
