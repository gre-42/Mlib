#pragma once
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Remote/Incremental_Objects/IIncremental_Object.hpp>
#include <Mlib/Remote/Remote_Site_Id.hpp>
#include <memory>

namespace Mlib {

class ObjectPool;
class PhysicsScene;
enum class IoVerbosity;

class RemoteUsers final: public IIncrementalObject {
public:
    explicit RemoteUsers(
        ObjectPool& object_pool,
        IoVerbosity verbosity,
        const DanglingBaseClassRef<PhysicsScene>& physics_scene,
        RemoteSiteId site_id);
    ~RemoteUsers();
    static std::unique_ptr<RemoteUsers> try_create_from_stream(
        ObjectPool& object_pool,
        PhysicsScene& physics_scene,
        std::istream& istr,
        IoVerbosity verbosity,
        RemoteSiteId site_id);
    virtual void read(std::istream& istr) override;
    virtual void write(std::ostream& ostr, ObjectCompression compression) override;

private:
    void read_data(std::istream& istr);
    std::string read_string(std::istream& istr, const char* msg) const;
    void write_string(std::ostream& ostr, const std::string& str, const char* msg) const;

    DanglingBaseClassRef<PhysicsScene> physics_scene_;
    IoVerbosity verbosity_;
    RemoteSiteId site_id_;
    DestructionFunctionsRemovalTokens physics_scene_on_destroy_;
};

}
