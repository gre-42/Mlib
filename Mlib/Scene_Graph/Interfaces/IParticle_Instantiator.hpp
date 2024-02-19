#pragma once
#include <cstddef>

namespace Mlib {

template <class TDir, class TPos, size_t n>
class TransformationMatrix;
enum class ParticleSubstrate;

class IParticleInstantiator {
public:
    virtual ~IParticleInstantiator() = default;
    virtual void add_particle(const TransformationMatrix<float, double, 3>& transformation_matrix) = 0;
    virtual ParticleSubstrate particle_substrate() const = 0;
    
};

}
