#pragma once
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Remote/Incremental_Objects/IIncremental_Object.hpp>
#include <memory>
#include <string>

namespace Mlib {

class PhysicsScene;
class Player;
class ObjectPool;
enum class IoVerbosity;

class RemotePlayer final: public IIncrementalObject {
public:
    explicit RemotePlayer(
        ObjectPool& object_pool,
        IoVerbosity verbosity,
        const DanglingBaseClassRef<Player>& player);
    ~RemotePlayer();
    static std::unique_ptr<RemotePlayer> try_create_from_stream(
        ObjectPool& object_pool,
        PhysicsScene& physics_scene,
        std::istream& istr,
        IoVerbosity verbosity);
    virtual void read(std::istream& istr) override;
    virtual void write(std::ostream& ostr, ObjectCompression compression) override;

private:
    DanglingBaseClassRef<Player> player_;
    IoVerbosity verbosity_;
    DestructionFunctionsRemovalTokens player_on_destroy_;
};

}
