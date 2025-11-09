#ifndef EC_RIGID_BODY_H
#define EC_RIGID_BODY_H

// Entity
#include "entity/entity.h"
#include "entity/transform.h"
// Collider
#include "entity/components/ec_collider/ec_collider.h"
// Physics
#include "physics/physics-manager.h"

// -------------------------
// Types
// -------------------------

typedef struct EC_Collider EC_Collider;

typedef struct RigidBodyConstraints
{
    bool freezePositionX;
    bool freezePositionY;
    bool freezePositionZ;
    bool freezeRotationX;
    bool freezeRotationY;
    bool freezeRotationZ;
} RigidBodyConstraints;

typedef struct EC_RigidBody
{
    Component *component;
    // Collider
    EC_Collider *ec_collider;
    float mass;
    bool useGravity;
    bool isKinematic;
    bool *isStatic;
    // Transform
    V3 w_pos;
    Quaternion w_rot;
    // Velocity
    V3 velocity;
    V3 angularVelocity;
    V3 forceAccum;
    V3 torqueAccum;
    float linearDamping;
    float angularDamping;
    // Sleeping system
    bool isSleeping;
    float sleepTimer;
    // Constraints
    RigidBodyConstraints constraints;
} EC_RigidBody;

// ------------------------- 
// Helper Functions 
// -------------------------

RigidBodyConstraints RigidBodyConstraints_None();
RigidBodyConstraints RigidBodyConstraints_FreezePosition();
RigidBodyConstraints RigidBodyConstraints_FreezeRotation();
RigidBodyConstraints RigidBodyConstraints_Humanoid();
// ------------------------- 
// Creation & Freeing 
// -------------------------

EC_RigidBody *EC_RigidBody_Create(Entity *entity, EC_Collider *ec_collider, float mass, bool useGravity, RigidBodyConstraints constraints);

#endif