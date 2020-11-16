#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Interfaces/Advance_Time.hpp>
#include <Mlib/Physics/Interfaces/External_Force_Provider.hpp>
#include <list>
#include <map>
#include <string>

namespace Mlib {

struct RigidBody;
struct RigidBodyIntegrator;
class Players;
class SceneNode;
class CollisionQuery;
class YawPitchLookAtNodes;
class Gun;

class Player: public DestructionObserver, public AdvanceTime, public ExternalForceProvider {
    friend class Players;
public:
    Player(
        CollisionQuery& collision_query,
        Players& players,
        const std::string& name,
        const std::string& team);
    void set_rigid_body(SceneNode& scene_node, RigidBody& rb);
    void set_ypln(YawPitchLookAtNodes& ypln, Gun* gun);
    void set_surface_power(float forward, float backward);
    void set_tire_angle_y(size_t tire_id, float angle_left, float angle_right);
    void set_waypoint(const FixedArray<float, 2>& waypoint);

    virtual void notify_destroyed(void* destroyed_object) override;
    virtual void advance_time(float dt) override;
    virtual void increment_external_forces(const std::list<std::shared_ptr<RigidBody>>& olist, bool burn_in, const PhysicsEngineConfig& cfg) override;
private:
    void select_opponent();
    void aim_and_shoot();
    void move_to_waypoint();
    CollisionQuery& collision_query_;
    Players& players_;
    std::string name_;
    std::string team_;
    SceneNode* scene_node_;
    SceneNode* target_scene_node_;
    RigidBody* rb_;
    RigidBodyIntegrator* target_rbi_;
    YawPitchLookAtNodes* ypln_;
    Gun* gun_;
    float surface_power_forward_;
    float surface_power_backward_;
    std::map<size_t, float> tire_angles_left_;
    std::map<size_t, float> tire_angles_right_;
    FixedArray<float, 2> waypoint_;
};

};
