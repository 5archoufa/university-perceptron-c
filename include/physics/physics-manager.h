#ifndef PHYSICS_MANAGER_H
#define PHYSICS_MANAGER_H

// Entity
#include "entity/entity.h"
typedef struct ELayer ELayer;
// Rigidbodies
#include "entity/components/ec_rigidbody/ec_rigidbody.h"
// Bounds
#include "physics/aabb.h"

typedef struct EC_RigidBody EC_RigidBody;
typedef struct EC_Collider EC_Collider;

// -------------------------
// Collision Events
// -------------------------

// Collision event data passed to callbacks
typedef struct CollisionInfo
{
    EC_Collider *collider;
    V3 contactPoint;
    V3 normal;
    float penetration;
} CollisionInfo;

// -------------------------
// Raycasting
// -------------------------

// Raycast hit information
typedef struct RaycastHit
{
    EC_Collider *collider;
    V3 point;
    V3 normal;
    float distance;
    bool hit;
} RaycastHit;

typedef struct Ray{
    V3 origin;
    V3 direction;
} Ray;

// -------------------------
// Types
// -------------------------

// Physics constants
#define PHYSICS_GRAVITY_EARTH -9.81f
#define PHYSICS_DEFAULT_LINEAR_DAMPING 0.99f
#define PHYSICS_DEFAULT_ANGULAR_DAMPING 0.95f

typedef struct Proxy
{
    uint32_t id;
    AABB aabb;
    Entity *entity;
    // Colision Mask?
    bool isStatic;
} Proxy;

typedef struct PhysicsManager
{
    size_t rigidbodies_size;
    EC_RigidBody **rigidbodies;
    Proxy *proxies;

    // Global physics settings
    V3 gravity;
    float timeStep;
} PhysicsManager;

// -------------------------
// Update Loop
// -------------------------

void PhysicsManager_FixedUpdate(PhysicsManager *physicsManager);

// -------------------------
// Manager Management
// -------------------------

void PhysicsManager_Select(PhysicsManager *physicsManager);

// -------------------------
// Physics Operations
// -------------------------

void PhysicsManager_AddForce(EC_RigidBody *rigidbody, V3 force);
void PhysicsManager_AddTorque(EC_RigidBody *rigidbody, V3 torque);
void PhysicsManager_SetVelocity(EC_RigidBody *rigidbody, V3 velocity);
void PhysicsManager_SetAngularVelocity(EC_RigidBody *rigidbody, V3 angularVelocity);

// -------------------------
// Physics Settings
// -------------------------

void PhysicsManager_SetGravity(V3 gravity);
V3 PhysicsManager_GetGravity();
void PhysicsManager_SetTimeStep(float timeStep);
float PhysicsManager_GetTimeStep();

// -------------------------
// Rigidbody Management
// -------------------------

void PhysicsManager_RegisterRigidBody(EC_RigidBody *ec_rigidbody);
void PhysicsManager_RemoveRigidBody(EC_RigidBody *ec_rigidbody);

// -------------------------
// Raycasting & Shape Casting
// -------------------------

/**
 * @brief Cast a ray and find the first collider it hits
 * @param origin Starting point of the ray in world space
 * @param direction Direction of the ray (will be normalized)
 * @param maxDistance Maximum distance to check
 * @param outHit Pointer to RaycastHit struct to fill with results
 * @param layerMask Bitmask of layers to include in the raycast
 * @return true if something was hit, false otherwise
 */
bool PhysicsManager_Raycast(Ray *ray, float maxDistance, RaycastHit *outHit, uint32_t layerMask);

/**
 * @brief Initialize all fields of a RaycastHit to default values
 * @param hit Pointer to the RaycastHit to initialize
 */
void RaycastHit_Init(RaycastHit *hit);

/**
 * @brief Find all colliders overlapping a box shape
 * @param center Center of the box in world space
 * @param halfExtents Half-size of the box on each axis
 * @param outColliders Array to store found colliders
 * @param maxColliders Maximum number of colliders to return
 * @return Number of colliders found
 */
int PhysicsManager_BoxCast(V3 center, V3 halfExtents, EC_Collider **outColliders, int maxColliders);

/**
 * @brief Find all colliders overlapping a sphere shape
 * @param center Center of the sphere in world space
 * @param radius Radius of the sphere
 * @param outColliders Array to store found colliders
 * @param maxColliders Maximum number of colliders to return
 * @return Number of colliders found
 */
int PhysicsManager_SphereCast(V3 center, float radius, EC_Collider **outColliders, int maxColliders);

// -------------------------
// Creation & Freeing
// -------------------------

PhysicsManager *PhysicsManager_Create(float timeStep);
void PhysicsManager_Select(PhysicsManager *physicsManager);
void PhysicsManager_Free(PhysicsManager *manager);

#endif