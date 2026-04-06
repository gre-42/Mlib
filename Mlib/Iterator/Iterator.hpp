#pragma once

namespace Mlib {

template <class T>
concept Iterator = requires(T x)
{
    ++x;
};

}
