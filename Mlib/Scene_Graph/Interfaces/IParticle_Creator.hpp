#pragma once
#include <Mlib/Scene_Pos.hpp>
#include <cstddef>

namespace Mlib {

template <class TDir, class TPos, size_t n>
class TransformationMatrix;

class IParticleCreator {
public:
    virtual ~IParticleCreator() = default;
    virtual void add_particle(const TransformationMatrix<float, ScenePos, 3>& transformation_matrix) = 0;
    
};

}
