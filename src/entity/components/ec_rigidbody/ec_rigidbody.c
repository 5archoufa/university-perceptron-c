#include "entity/components/ec_rigidbody/ec_rigidbody.h"

// -------------------------
// Helper Functions
// -------------------------

RigidBodyConstraints RigidBodyConstraints_Humanoid()
{
    return (RigidBodyConstraints){
        .freezePositionX = true,
        .freezePositionY = true,
        .freezePositionZ = true,
        .freezeRotationX = false,
        .freezeRotationY = true,
        .freezeRotationZ = false};
}

RigidBodyConstraints RigidBodyConstraints_FreezePosition()
{
    return (RigidBodyConstraints){
        .freezePositionX = true,
        .freezePositionY = true,
        .freezePositionZ = true,
        .freezeRotationX = false,
        .freezeRotationY = false,
        .freezeRotationZ = false};
}

RigidBodyConstraints RigidBodyConstraints_None()
{
    return (RigidBodyConstraints){
        .freezePositionX = false,
        .freezePositionY = false,
        .freezePositionZ = false,
        .freezeRotationX = false,
        .freezeRotationY = false,
        .freezeRotationZ = false};
}

RigidBodyConstraints RigidBodyConstraints_FreezeRotation()
{
    return (RigidBodyConstraints){
        .freezePositionX = false,
        .freezePositionY = false,
        .freezePositionZ = false,
        .freezeRotationX = true,
        .freezeRotationY = true,
        .freezeRotationZ = true};
}

// -------------------------
// Creation & Freeing
// -------------------------

EC_RigidBody *EC_RigidBody_Create(Entity *entity, EC_Collider *ec_collider, float mass, bool useGravity, RigidBodyConstraints constraints)
{
    EC_RigidBody *ec_rigidbody = malloc(sizeof(EC_RigidBody));
    ec_rigidbody->ec_collider = ec_collider;
    ec_rigidbody->mass = mass;
    ec_rigidbody->useGravity = useGravity;
    ec_rigidbody->isKinematic = false;
    ec_rigidbody->isStatic = &entity->isStatic;
    // Transform
    ec_rigidbody->w_pos = T_WPos(&entity->transform);
    ec_rigidbody->w_rot = T_WRot(&entity->transform);
    // Velocity
    ec_rigidbody->velocity = V3_ZERO;
    ec_rigidbody->angularVelocity = V3_ZERO;
    ec_rigidbody->forceAccum = V3_ZERO;
    ec_rigidbody->torqueAccum = V3_ZERO;
    ec_rigidbody->linearDamping = 0.99f;
    ec_rigidbody->angularDamping = 0.99f;
    // Constraints
    ec_rigidbody->constraints = constraints;
    // Component
    ec_rigidbody->component = Component_Create(ec_rigidbody, entity, EC_T_RIGIDBODY, NULL, NULL, NULL, NULL, NULL, NULL);
    // Register with Physics Manager
    //PhysicsManager_RegisterRigidBody(ec_rigidbody);
    return ec_rigidbody;
}