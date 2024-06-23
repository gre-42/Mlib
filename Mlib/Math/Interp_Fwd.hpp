#pragma once

namespace Mlib {

template <class T>
class DefaultUnitialized;

template <class TDataX, class TDataY = TDataX>
class Interp;

template <class TDataX, class TDataY>
using UUInterp = Interp<TDataX, DefaultUnitialized<TDataY>>;

}
