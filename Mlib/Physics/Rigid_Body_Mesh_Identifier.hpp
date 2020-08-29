#pragma once
#include <memory>

namespace Mlib {

class RigidBody;

struct RigidBodyMeshIdentifier {
    std::shared_ptr<RigidBody> rigid_body;
    size_t mesh_id;
};

}
