#pragma once
#include <cstddef>

namespace Mlib {

template <class TDir, class TPos, size_t n>
class TransformationMatrix;

class IParticleCreator {
public:
    virtual ~IParticleCreator() = default;
    virtual void add_particle(const TransformationMatrix<float, double, 3>& transformation_matrix) = 0;
    
};

}
