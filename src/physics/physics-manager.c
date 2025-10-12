#include "physics/physics-manager.h"
#include "perceptron.h"
// Entity
#include "entity/entity.h"
// Rigidbody
#include "entity/components/ec_rigidbody/ec_rigidbody.h"
// C
#include <stdlib.h>
// Logging
#include "logging/logger.h"

// -------------------------
// Static Variables
// -------------------------

static LogConfig _logConfig = {"PhysicsManager", LOG_LEVEL_INFO, LOG_COLOR_BLUE};
static PhysicsManager *_manager;

// -------------------------
// Utilities
// -------------------------

inline static bool AABB_Overlap(AABB a, AABB b)
{
    return (a.min.x <= b.max.x && a.max.x >= b.min.x) &&
           (a.min.y <= b.max.y && a.max.y >= b.min.y) &&
           (a.min.z <= b.max.z && a.max.z >= b.min.z);
}

inline static void UpdateWorldAABB(EC_Collider *collider)
{
    if (!collider || !collider->transform)
        return;

    Transform *transform = collider->transform;
    V3 worldPos = T_WPos(transform);
    V3 worldScale = T_WSca(transform);
    V3 right = T_Right(transform);
    V3 up = T_Up(transform);
    V3 forward = T_Forward(transform);

    if (collider->type == EC_COLLIDER_SPHERE)
    {
        // Sphere: rotation doesn’t matter, just scale the radius
        float maxScale = fmaxf(fmaxf(worldScale.x, worldScale.y), worldScale.z);
        float worldRadius = collider->data.sphere.radius * maxScale;

        // Rotate offset into world space
        V3 worldOffset = {
            collider->offset.x * right.x + collider->offset.y * up.x + collider->offset.z * forward.x,
            collider->offset.x * right.y + collider->offset.y * up.y + collider->offset.z * forward.y,
            collider->offset.x * right.z + collider->offset.y * up.z + collider->offset.z * forward.z};

        V3 center = V3_ADD(worldPos, worldOffset);

        collider->worldAABB.min = V3_SUB(center, (V3){worldRadius, worldRadius, worldRadius});
        collider->worldAABB.max = V3_ADD(center, (V3){worldRadius, worldRadius, worldRadius});
    }
    else
    {
        // Box, Capsule, Mesh: rotation-aware AABB
        V3 localCenter = V3_SCALE(V3_ADD(collider->localAABB.min, collider->localAABB.max), 0.5f);
        V3 localHalf = V3_SCALE(V3_SUB(collider->localAABB.max, collider->localAABB.min), 0.5f);

        // Apply entity scale
        localCenter = V3_MUL(localCenter, worldScale);
        localHalf = V3_ABS(V3_MUL(localHalf, worldScale));

        // Rotate offset into world space
        V3 worldOffset = {
            collider->offset.x * right.x + collider->offset.y * up.x + collider->offset.z * forward.x,
            collider->offset.x * right.y + collider->offset.y * up.y + collider->offset.z * forward.y,
            collider->offset.x * right.z + collider->offset.y * up.z + collider->offset.z * forward.z};

        // Compute world half-extents
        V3 absRight = {fabsf(right.x), fabsf(right.y), fabsf(right.z)};
        V3 absUp = {fabsf(up.x), fabsf(up.y), fabsf(up.z)};
        V3 absForward = {fabsf(forward.x), fabsf(forward.y), fabsf(forward.z)};
        V3 halfWorld = {
            localHalf.x * absRight.x + localHalf.y * absUp.x + localHalf.z * absForward.x,
            localHalf.x * absRight.y + localHalf.y * absUp.y + localHalf.z * absForward.y,
            localHalf.x * absRight.z + localHalf.y * absUp.z + localHalf.z * absForward.z};

        // Final world-space center
        V3 center = V3_ADD(worldPos, V3_ADD(worldOffset, localCenter));

        collider->worldAABB.min = V3_SUB(center, halfWorld);
        collider->worldAABB.max = V3_ADD(center, halfWorld);
    }
}

inline static void BroadPhase()
{
    EC_RigidBody *ec_rigidbody = NULL;
    for (int i = 0; i < _manager->rigid_bodies_size; i++)
    {
        if (!_manager->rigid_bodies[i])
        {
            continue;
        }
        ec_rigidbody = _manager->rigid_bodies[i];
        if (!ec_rigidbody->isStatic)
        {
            UpdateWorldAABB(ec_rigidbody->ec_collider);
        }
    }
}

inline static void NarrowPhase()
{
}

// -------------------------
// Management
// -------------------------

void PhysicsManager_RegisterRigidBody(EC_RigidBody *ec_rigidbody)
{
    for (int i = 0; i < _manager->rigid_bodies_size; i++)
    {
        if (_manager->rigid_bodies[i] == NULL)
        {
            _manager->rigid_bodies[i] = ec_rigidbody;
            LogSuccess(&_logConfig, "Registered Rigidbody. Total Rigidbodies: %d", _manager->rigid_bodies_size);
            return;
        }
    }
    _manager->rigid_bodies_size++;
    _manager->rigid_bodies = realloc(_manager->rigid_bodies, sizeof(EC_RigidBody *) * _manager->rigid_bodies_size);
    _manager->rigid_bodies[_manager->rigid_bodies_size - 1] = ec_rigidbody;
    LogSuccess(&_logConfig, "Registered Rigidbody. Total Rigidbodies: %d", _manager->rigid_bodies_size);
}

void PhysicsManager_RemoveRigidBody(EC_RigidBody *ec_rigidbody)
{
    for (int i = 0; i < _manager->rigid_bodies_size; i++)
    {
        if (_manager->rigid_bodies[i] == ec_rigidbody)
        {
            _manager->rigid_bodies[i] = NULL;
            _manager->rigid_bodies_size--;
            LogSuccess(&_logConfig, "Removed Rigidbody. Total Rigidbodies: %d", _manager->rigid_bodies_size);
            return;
        }
    }
    LogError(&_logConfig, "Failed to remove Rigidbody (%s), not found.", ec_rigidbody->component->entity->name);
}

// -------------------------
// Functions
// -------------------------

void FixedUpdate(float fixedDeltaTime)
{
    BroadPhase();
    NarrowPhase();
}

// -------------------------
// Manager Management
// -------------------------

void PhysicsManager_Select(PhysicsManager *physicsManager)
{
    _manager = physicsManager;
    LogSuccess(&_logConfig, "Selected Physics Manager.");
}

// -------------------------
// Layers Filter
// -------------------------

static void PhysicsManager_FreeLayerFilter()
{
    for (int i = 0; i < _manager->_elayers_Size; i++)
    {
        free(_manager->_elayerFilter[i]);
    }
    free(_manager->_elayerFilter);
}

// -------------------------
// Creation & Freeing
// -------------------------

PhysicsManager *PhysicsManager_Create()
{
    PhysicsManager *manager = malloc(sizeof(PhysicsManager));
    manager->rigid_bodies_size = 0;
    manager->rigid_bodies = NULL;
    manager->_elayers_Size = ELAYERS_SIZE;
    LogCreate(&_logConfig, "");
    PhysicsManager_Select(manager);
    // Setup Entity-Layers Filter
    size_t elayers_size = ELAYERS_SIZE;
    LogSuccess(&_logConfig, "Setting Physics Layer Filter with %d layers:", elayers_size);
    bool filter[ELAYERS_SIZE][ELAYERS_SIZE] = ELAYERS_PHYSICS_FILTER;
    manager->_elayers_Size = elayers_size;
    manager->_elayerFilter = malloc(sizeof(bool *) * elayers_size);
    for (int i = 0; i < elayers_size; i++)
    {
        manager->_elayerFilter[i] = malloc(sizeof(bool) * elayers_size);
        for (int j = 0; j < elayers_size; j++)
        {
            manager->_elayerFilter[i][j] = filter[i][j];
        }
    }
    return manager;
}

void PhysicsManager_Free(PhysicsManager *manager)
{
    PhysicsManager_Select(manager);
    // Layer filter
    PhysicsManager_FreeLayerFilter();
    // Rigidbodies
    free(manager->rigid_bodies);
    free(manager);
    LogFree(&_logConfig, "");
}