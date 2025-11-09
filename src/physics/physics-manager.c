#include "physics/physics-manager.h"
#include "physics/mesh_bvh.h"
#include "perceptron.h"
// Entity
#include "entity/entity.h"
// Rigidbody
#include "entity/components/ec_rigidbody/ec_rigidbody.h"
// Collider
#include "entity/components/ec_collider/ec_collider.h"
// C
#include <stdlib.h>
#include <math.h>
// Logging
#include "logging/logger.h"

// -------------------------
// Static Variables
// -------------------------

static LogConfig _logConfig = {"PhysicsManager", LOG_LEVEL_INFO, LOG_COLOR_BLUE};
static PhysicsManager *_manager;

static const uint32_t COLLISION_MASK[] = {
    [E_LAYER_DEFAULT] = (1u << E_LAYER_DEFAULT) | (1u << E_LAYER_CREATURE) | (1u << E_LAYER_RAYCAST) | (1u << E_LAYER_TERRAIN),
    [E_LAYER_CREATURE] = (1u << E_LAYER_DEFAULT) | (1u << E_LAYER_CREATURE) | (1u << E_LAYER_TERRAIN) | (1u << E_LAYER_RAYCAST),
    [E_LAYER_RAYCAST] = (1u << E_LAYER_DEFAULT) | (1u << E_LAYER_CREATURE),
    [E_LAYER_TERRAIN] = (1u << E_LAYER_DEFAULT) | (1u << E_LAYER_CREATURE) | (1u << E_LAYER_RAYCAST),
    [E_LAYER_TRIGLE] = (1u << E_LAYER_TRIGLE),
    [E_LAYER_GUI] = 0u,
};

// -------------------------
// Sleeping & Contact Constants
// -------------------------

#define SLEEP_VELOCITY_THRESHOLD 0.005f // Lower threshold for better stability
#define SLEEP_ANGULAR_THRESHOLD 0.005f  // Separate threshold for rotation
#define SLEEP_TIME_THRESHOLD 0.3f       // Faster sleep time
#define WAKE_VELOCITY_THRESHOLD 0.1f
#define RESTING_VELOCITY_DAMPING 0.90f // Stronger damping for resting

// Position correction to prevent penetration accumulation
#define PENETRATION_ALLOWANCE 0.01f // Allow small overlap (slop)
#define PENETRATION_CORRECTION 0.4f // Percentage to correct per frame (Baumgarte stabilization)

// Contact properties
#define STATIC_FRICTION_THRESHOLD 0.5f // Velocity threshold for static friction
#define STATIC_FRICTION_COEF 0.6f      // Static friction coefficient
#define DYNAMIC_FRICTION_COEF 0.4f     // Dynamic friction coefficient

// -------------------------
// Collision Tracking Helpers
// -------------------------

// Helper to check if collider is already in collision list
inline static bool IsInCollisionList(EC_Collider *collider, EC_Collider *other)
{
    for (int i = 0; i < collider->currentCollisions_size; i++)
    {
        if (collider->currentCollisions[i] == other)
            return true;
    }
    return false;
}

// Helper to add collider to collision list
inline static void AddToCollisionList(EC_Collider *collider, EC_Collider *other)
{
    // Check if we need to expand capacity
    if (collider->currentCollisions_size >= collider->currentCollisions_capacity)
    {
        collider->currentCollisions_capacity = (collider->currentCollisions_capacity == 0) ? 4 : collider->currentCollisions_capacity * 2;
        collider->currentCollisions = realloc(collider->currentCollisions,
                                              collider->currentCollisions_capacity * sizeof(EC_Collider *));
    }

    collider->currentCollisions[collider->currentCollisions_size++] = other;
}

// Helper to remove collider from collision list
inline static void RemoveFromCollisionList(EC_Collider *collider, EC_Collider *other)
{
    for (int i = 0; i < collider->currentCollisions_size; i++)
    {
        if (collider->currentCollisions[i] == other)
        {
            // Shift remaining elements
            for (int j = i; j < collider->currentCollisions_size - 1; j++)
            {
                collider->currentCollisions[j] = collider->currentCollisions[j + 1];
            }
            collider->currentCollisions_size--;
            return;
        }
    }
}

// -------------------------
// Enhanced Collision Response with Events
// -------------------------

static void HandleCollision(EC_RigidBody *bodyA, EC_RigidBody *bodyB, EC_Collider *colliderA, EC_Collider *colliderB)
{
    // Wake up sleeping bodies
    if (bodyA && bodyA->isSleeping)
    {
        bodyA->isSleeping = false;
        bodyA->sleepTimer = 0.0f;
    }
    if (bodyB && bodyB->isSleeping)
    {
        bodyB->isSleeping = false;
        bodyB->sleepTimer = 0.0f;
    }

    AABB aabbA = colliderA->worldAABB;
    AABB aabbB = colliderB->worldAABB;

    V3 centerA = V3_SCALE(V3_ADD(aabbA.min, aabbA.max), 0.5f);
    V3 centerB = V3_SCALE(V3_ADD(aabbB.min, aabbB.max), 0.5f);

    // Calculate overlap
    V3 sizeA = V3_SUB(aabbA.max, aabbA.min);
    V3 sizeB = V3_SUB(aabbB.max, aabbB.min);

    float overlapX = (sizeA.x + sizeB.x) * 0.5f - fabsf(centerB.x - centerA.x);
    float overlapY = (sizeA.y + sizeB.y) * 0.5f - fabsf(centerB.y - centerA.y);
    float overlapZ = (sizeA.z + sizeB.z) * 0.5f - fabsf(centerB.z - centerA.z);

    // Find minimum overlap axis
    V3 normal = V3_ZERO;
    float penetration = 0.0f;
    V3 contactPoint = V3_ZERO;

    if (overlapX <= overlapY && overlapX <= overlapZ)
    {
        normal = (V3){(centerB.x > centerA.x) ? 1.0f : -1.0f, 0, 0};
        penetration = overlapX;
        float contactX = (centerB.x > centerA.x) ? (aabbA.max.x) : (aabbA.min.x);
        contactPoint = (V3){contactX, (centerA.y + centerB.y) * 0.5f, (centerA.z + centerB.z) * 0.5f};
    }
    else if (overlapY <= overlapX && overlapY <= overlapZ)
    {
        normal = (V3){0, (centerB.y > centerA.y) ? 1.0f : -1.0f, 0};
        penetration = overlapY;
        float contactY = (centerB.y > centerA.y) ? (aabbA.max.y) : (aabbA.min.y);
        contactPoint = (V3){(centerA.x + centerB.x) * 0.5f, contactY, (centerA.z + centerB.z) * 0.5f};
    }
    else
    {
        normal = (V3){0, 0, (centerB.z > centerA.z) ? 1.0f : -1.0f};
        penetration = overlapZ;
        float contactZ = (centerB.z > centerA.z) ? (aabbA.max.z) : (aabbA.min.z);
        contactPoint = (V3){(centerA.x + centerB.x) * 0.5f, (centerA.y + centerB.y) * 0.5f, contactZ};
    }

    if (penetration <= 0.0f)
        return;

    // === COLLISION EVENT HANDLING ===
    bool wasCollidingA = IsInCollisionList(colliderA, colliderB);
    bool wasCollidingB = IsInCollisionList(colliderB, colliderA);

    // Prepare collision info for callbacks
    CollisionInfo infoForA = {
        .collider = colliderB,
        .contactPoint = contactPoint,
        .normal = V3_SCALE(normal, -1.0f), // Normal points away from A
        .penetration = penetration};

    CollisionInfo infoForB = {
        .collider = colliderA,
        .contactPoint = contactPoint,
        .normal = normal, // Normal points away from B
        .penetration = penetration};

    // Fire OnCollisionEnter or OnCollisionStay
    if (!wasCollidingA)
    {
        AddToCollisionList(colliderA, colliderB);
        if (colliderA->OnCollisionEnter)
            colliderA->OnCollisionEnter(colliderA, &infoForA);
    }
    else
    {
        if (colliderA->OnCollisionStay)
            colliderA->OnCollisionStay(colliderA, &infoForA);
    }

    if (!wasCollidingB)
    {
        AddToCollisionList(colliderB, colliderA);
        if (colliderB->OnCollisionEnter)
            colliderB->OnCollisionEnter(colliderB, &infoForB);
    }
    else
    {
        if (colliderB->OnCollisionStay)
            colliderB->OnCollisionStay(colliderB, &infoForB);
    }

    // === PHYSICS RESPONSE (skip if either is trigger) ===
    if (colliderA->isTrigger || colliderB->isTrigger)
        return;

    float massA = (*colliderA->isStatic || bodyA->mass <= 0) ? 0.0f : bodyA->mass;
    float massB = (*colliderB->isStatic || bodyB->mass <= 0) ? 0.0f : bodyB->mass;

    // Calculate inverse masses
    float invMassA = (massA > 0.0f) ? 1.0f / massA : 0.0f;
    float invMassB = (massB > 0.0f) ? 1.0f / massB : 0.0f;
    float totalInvMass = invMassA + invMassB;

    if (totalInvMass <= 0.0f)
        return; // Both static

    // ===== POSITION CORRECTION (prevent sinking) =====
    float correctionAmount = fmaxf(penetration - PENETRATION_ALLOWANCE, 0.0f) * PENETRATION_CORRECTION;
    V3 correction = V3_SCALE(normal, correctionAmount);

    if (!*colliderA->isStatic)
    {
        Transform *transformA = &bodyA->component->entity->transform;
        V3 correctionA = V3_SCALE(correction, -invMassA / totalInvMass);
        T_LPos_Add(transformA, correctionA);
    }

    if (!*colliderB->isStatic)
    {
        Transform *transformB = &bodyB->component->entity->transform;
        V3 correctionB = V3_SCALE(correction, invMassB / totalInvMass);
        T_LPos_Add(transformB, correctionB);
    }

    // ===== CALCULATE CONTACT POINT RELATIVE TO CENTER OF MASS =====
    V3 rA = V3_SUB(contactPoint, centerA); // Vector from A's center to contact
    V3 rB = V3_SUB(contactPoint, centerB); // Vector from B's center to contact

    // ===== CALCULATE RELATIVE VELOCITY AT CONTACT POINT =====
    // For rigid body: velocity_at_point = linear_velocity + (angular_velocity × r)
    V3 velocityA = bodyA->velocity;
    V3 velocityB = bodyB->velocity;

    if (!*colliderA->isStatic)
    {
        V3 rotationalVelA = V3_CROSS(bodyA->angularVelocity, rA);
        velocityA = V3_ADD(velocityA, rotationalVelA);
    }

    if (!*colliderB->isStatic)
    {
        V3 rotationalVelB = V3_CROSS(bodyB->angularVelocity, rB);
        velocityB = V3_ADD(velocityB, rotationalVelB);
    }

    V3 relativeVelocity = V3_SUB(velocityB, velocityA);
    float velocityAlongNormal = V3_DOT(relativeVelocity, normal);

    // Objects separating - no impulse needed
    if (velocityAlongNormal > 0.0f)
        return;

    // Determine restitution (bounciness)
    float restitution = (fabsf(velocityAlongNormal) < SLEEP_VELOCITY_THRESHOLD) ? 0.0f : 0.3f;

    // ===== CALCULATE ANGULAR CONTRIBUTION TO IMPULSE =====
    // Simplified inertia: treat as uniform box
    V3 invInertiaA = V3_ZERO;
    V3 invInertiaB = V3_ZERO;

    if (!*colliderA->isStatic && massA > 0.0f)
    {
        float sizeSquared = (sizeA.x * sizeA.x + sizeA.y * sizeA.y + sizeA.z * sizeA.z) / 6.0f;
        float inertia = massA * sizeSquared;
        if (inertia > 0.0f)
        {
            invInertiaA = (V3){1.0f / inertia, 1.0f / inertia, 1.0f / inertia};
        }
    }

    if (!*colliderB->isStatic && massB > 0.0f)
    {
        float sizeSquared = (sizeB.x * sizeB.x + sizeB.y * sizeB.y + sizeB.z * sizeB.z) / 6.0f;
        float inertia = massB * sizeSquared;
        if (inertia > 0.0f)
        {
            invInertiaB = (V3){1.0f / inertia, 1.0f / inertia, 1.0f / inertia};
        }
    }

    // Calculate denominators for impulse equation including rotation
    V3 rA_cross_n = V3_CROSS(rA, normal);
    V3 rB_cross_n = V3_CROSS(rB, normal);

    V3 angularContribA = V3_CROSS(V3_MUL(rA_cross_n, invInertiaA), rA);
    V3 angularContribB = V3_CROSS(V3_MUL(rB_cross_n, invInertiaB), rB);

    float angularEffect = V3_DOT(angularContribA, normal) + V3_DOT(angularContribB, normal);

    // Calculate impulse magnitude with angular effects
    float impulseMagnitude = -(1.0f + restitution) * velocityAlongNormal;
    impulseMagnitude /= (totalInvMass + angularEffect);

    V3 impulse = V3_SCALE(normal, impulseMagnitude);

    // ===== APPLY LINEAR AND ANGULAR IMPULSES =====
    if (!*colliderA->isStatic)
    {
        // Apply linear impulse
        bodyA->velocity = V3_SUB(bodyA->velocity, V3_SCALE(impulse, invMassA));

        // Apply angular impulse: Δω = I⁻¹ * (r × impulse)
        V3 torqueImpulse = V3_CROSS(rA, V3_SCALE(impulse, -1.0f));
        V3 angularImpulse = V3_MUL(torqueImpulse, invInertiaA);
        bodyA->angularVelocity = V3_ADD(bodyA->angularVelocity, angularImpulse);

        // Clamp very small velocities to zero
        if (V3_MAGNITUDE(bodyA->velocity) < SLEEP_VELOCITY_THRESHOLD)
        {
            bodyA->velocity = V3_ZERO;
        }
        if (V3_MAGNITUDE(bodyA->angularVelocity) < SLEEP_ANGULAR_THRESHOLD)
        {
            bodyA->angularVelocity = V3_ZERO;
        }
    }

    if (!*colliderB->isStatic)
    {
        // Apply linear impulse
        bodyB->velocity = V3_ADD(bodyB->velocity, V3_SCALE(impulse, invMassB));

        // Apply angular impulse
        V3 torqueImpulse = V3_CROSS(rB, impulse);
        V3 angularImpulse = V3_MUL(torqueImpulse, invInertiaB);
        bodyB->angularVelocity = V3_ADD(bodyB->angularVelocity, angularImpulse);

        if (V3_MAGNITUDE(bodyB->velocity) < SLEEP_VELOCITY_THRESHOLD)
        {
            bodyB->velocity = V3_ZERO;
        }
        if (V3_MAGNITUDE(bodyB->angularVelocity) < SLEEP_ANGULAR_THRESHOLD)
        {
            bodyB->angularVelocity = V3_ZERO;
        }
    }

    // ===== FRICTION WITH ANGULAR EFFECTS =====
    V3 tangent = V3_SUB(relativeVelocity, V3_SCALE(normal, velocityAlongNormal));
    float tangentLength = V3_MAGNITUDE(tangent);

    if (tangentLength > 0.001f)
    {
        tangent = V3_SCALE(tangent, 1.0f / tangentLength);

        float frictionCoef = (tangentLength < STATIC_FRICTION_THRESHOLD) ? STATIC_FRICTION_COEF : DYNAMIC_FRICTION_COEF;

        // Calculate friction with angular contribution
        V3 rA_cross_t = V3_CROSS(rA, tangent);
        V3 rB_cross_t = V3_CROSS(rB, tangent);

        V3 angularContribA_friction = V3_CROSS(V3_MUL(rA_cross_t, invInertiaA), rA);
        V3 angularContribB_friction = V3_CROSS(V3_MUL(rB_cross_t, invInertiaB), rB);

        float angularEffect_friction = V3_DOT(angularContribA_friction, tangent) +
                                       V3_DOT(angularContribB_friction, tangent);

        float frictionMagnitude = -V3_DOT(relativeVelocity, tangent);
        frictionMagnitude /= (totalInvMass + angularEffect_friction);

        // Clamp friction
        float maxFriction = fabsf(impulseMagnitude) * frictionCoef;
        frictionMagnitude = fmaxf(-maxFriction, fminf(frictionMagnitude, maxFriction));

        V3 frictionImpulse = V3_SCALE(tangent, frictionMagnitude);

        // Apply friction impulses
        if (!*colliderA->isStatic)
        {
            bodyA->velocity = V3_SUB(bodyA->velocity, V3_SCALE(frictionImpulse, invMassA));
            V3 frictionTorque = V3_CROSS(rA, V3_SCALE(frictionImpulse, -1.0f));
            bodyA->angularVelocity = V3_ADD(bodyA->angularVelocity, V3_MUL(frictionTorque, invInertiaA));
        }
        if (!*colliderB->isStatic)
        {
            bodyB->velocity = V3_ADD(bodyB->velocity, V3_SCALE(frictionImpulse, invMassB));
            V3 frictionTorque = V3_CROSS(rB, frictionImpulse);
            bodyB->angularVelocity = V3_ADD(bodyB->angularVelocity, V3_MUL(frictionTorque, invInertiaB));
        }
    }
}

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
        // Extract half-size directly from the local AABB (already includes offset)
        V3 localCenter = V3_SCALE(V3_ADD(collider->localAABB.min, collider->localAABB.max), 0.5f);
        V3 localHalf = V3_SCALE(V3_SUB(collider->localAABB.max, collider->localAABB.min), 0.5f);

        // Apply entity scale
        localCenter = V3_MUL(localCenter, worldScale);
        localHalf = V3_ABS(V3_MUL(localHalf, worldScale));

        // Compute world half-extents (rotation-aware)
        V3 absRight = {fabsf(right.x), fabsf(right.y), fabsf(right.z)};
        V3 absUp = {fabsf(up.x), fabsf(up.y), fabsf(up.z)};
        V3 absForward = {fabsf(forward.x), fabsf(forward.y), fabsf(forward.z)};
        V3 halfWorld = {
            localHalf.x * absRight.x + localHalf.y * absUp.x + localHalf.z * absForward.x,
            localHalf.x * absRight.y + localHalf.y * absUp.y + localHalf.z * absForward.y,
            localHalf.x * absRight.z + localHalf.y * absUp.z + localHalf.z * absForward.z};

        // Final world-space center (localCenter already includes the offset, just add worldPos)
        V3 center = V3_ADD(worldPos, localCenter);

        collider->worldAABB.min = V3_SUB(center, halfWorld);
        collider->worldAABB.max = V3_ADD(center, halfWorld);
    }
}

inline static void BroadPhase()
{
    EC_RigidBody *ec_rigidbody = NULL;
    for (int i = 0; i < _manager->rigidbodies_size; i++)
    {
        if (!_manager->rigidbodies[i])
        {
            continue;
        }
        ec_rigidbody = _manager->rigidbodies[i];
        if (!*ec_rigidbody->isStatic)
        {
            UpdateWorldAABB(ec_rigidbody->ec_collider);
        }
    }
}

inline static void DetectCollisionExits()
{
    // Check all colliders for collisions that have ended
    for (int i = 0; i < _manager->rigidbodies_size; i++)
    {
        if (!_manager->rigidbodies[i])
            continue;

        EC_RigidBody *bodyA = _manager->rigidbodies[i];
        EC_Collider *colliderA = bodyA->ec_collider;
        if (!colliderA)
            continue;

        // Check each collision in the list
        for (int j = 0; j < colliderA->currentCollisions_size; j++)
        {
            EC_Collider *colliderB = colliderA->currentCollisions[j];

            // Check if they're still colliding
            if (!AABB_Overlap(colliderA->worldAABB, colliderB->worldAABB))
            {
                // Collision ended - fire OnCollisionExit
                if (colliderA->OnCollisionExit)
                {
                    CollisionInfo info = {
                        .collider = colliderB,
                        .contactPoint = V3_ZERO,
                        .normal = V3_ZERO,
                        .penetration = 0.0f};
                    colliderA->OnCollisionExit(colliderA, &info);
                }

                // Remove from list
                RemoveFromCollisionList(colliderA, colliderB);
                j--; // Adjust index since we removed an element
            }
        }
    }
}

inline static void NarrowPhase()
{
    // Perform detailed collision detection between overlapping AABBs
    for (int i = 0; i < _manager->rigidbodies_size; i++)
    {
        if (!_manager->rigidbodies[i])
            continue;

        EC_RigidBody *bodyA = _manager->rigidbodies[i];
        EC_Collider *colliderA = bodyA->ec_collider;
        if (!colliderA)
            continue;

        for (int j = i + 1; j < _manager->rigidbodies_size; j++)
        {
            if (!_manager->rigidbodies[j])
                continue;

            EC_RigidBody *bodyB = _manager->rigidbodies[j];
            EC_Collider *colliderB = bodyB->ec_collider;
            if (!colliderB)
                continue;
            // Skip if both bodies are static
            if (*colliderA->isStatic && *colliderB->isStatic)
                continue;
            // Skip if layers do not collide
            if (!(COLLISION_MASK[colliderA->component->entity->layer] & (1 << colliderB->component->entity->layer)))
                continue;

            // Check for AABB overlap first
            if (AABB_Overlap(colliderA->worldAABB, colliderB->worldAABB))
            {
                // Only log collision once per second to avoid spam
                static float lastLogTime = 0.0f;
                static float currentTime = 0.0f;
                currentTime += 0.016f; // Approximate frame time

                if (currentTime - lastLogTime > 1.0f)
                {
                    // Log(&_logConfig, "Collision detected between %s and %s",
                    //     colliderA->component->entity->name,
                    //     colliderB->component->entity->name);
                    lastLogTime = currentTime;
                }

                // Generate collision response
                HandleCollision(bodyA, bodyB, colliderA, colliderB);
            }
        }
    }

    // Detect collision exits for all rigidbodies
    DetectCollisionExits();
}

// -------------------------
// Management
// -------------------------

void PhysicsManager_RegisterRigidBody(EC_RigidBody *ec_rigidbody)
{
    for (int i = 0; i < _manager->rigidbodies_size; i++)
    {
        if (_manager->rigidbodies[i] == NULL)
        {
            _manager->rigidbodies[i] = ec_rigidbody;
            LogSuccess(&_logConfig, "Registered Rigidbody. Total Rigidbodies: %d", _manager->rigidbodies_size);
            return;
        }
    }
    _manager->rigidbodies_size++;
    _manager->rigidbodies = realloc(_manager->rigidbodies, sizeof(EC_RigidBody *) * _manager->rigidbodies_size);
    _manager->rigidbodies[_manager->rigidbodies_size - 1] = ec_rigidbody;
    LogSuccess(&_logConfig, "Registered Rigidbody. Total Rigidbodies: %d", _manager->rigidbodies_size);
}

void PhysicsManager_RemoveRigidBody(EC_RigidBody *ec_rigidbody)
{
    for (int i = 0; i < _manager->rigidbodies_size; i++)
    {
        if (_manager->rigidbodies[i] == ec_rigidbody)
        {
            _manager->rigidbodies[i] = NULL;
            return;
        }
    }
    LogWarning(&_logConfig, "Failed to remove Rigidbody (%s), not found.", ec_rigidbody->component->entity->name);
}

// -------------------------
// Physics Operations
// -------------------------

void PhysicsManager_AddForce(EC_RigidBody *ec_rigidbody, V3 force)
{
    if (!ec_rigidbody || ec_rigidbody->isKinematic || *ec_rigidbody->isStatic)
        return;

    ec_rigidbody->forceAccum = V3_ADD(ec_rigidbody->forceAccum, force);
}

void PhysicsManager_AddTorque(EC_RigidBody *ec_rigidbody, V3 torque)
{
    if (!ec_rigidbody || ec_rigidbody->isKinematic || *ec_rigidbody->isStatic)
        return;

    ec_rigidbody->torqueAccum = V3_ADD(ec_rigidbody->torqueAccum, torque);
}

void PhysicsManager_SetVelocity(EC_RigidBody *ec_rigidbody, V3 velocity)
{
    if (!ec_rigidbody || ec_rigidbody->isKinematic || *ec_rigidbody->isStatic)
        return;

    ec_rigidbody->velocity = velocity;
}

void PhysicsManager_SetAngularVelocity(EC_RigidBody *ec_rigidbody, V3 angularVelocity)
{
    if (!ec_rigidbody || ec_rigidbody->isKinematic || *ec_rigidbody->isStatic)
        return;

    ec_rigidbody->angularVelocity = angularVelocity;
}

// -------------------------
// Physics Settings
// -------------------------

void PhysicsManager_SetGravity(V3 gravity)
{
    if (_manager)
        _manager->gravity = gravity;
}

V3 PhysicsManager_GetGravity()
{
    if (_manager)
        return _manager->gravity;
    return V3_ZERO;
}

void PhysicsManager_SetTimeStep(float timeStep)
{
    if (_manager)
        _manager->timeStep = timeStep;
}

float PhysicsManager_GetTimeStep()
{
    if (_manager)
        return _manager->timeStep;
    return 1.0f / 60.0f;
}

// -------------------------
// Raycasting & Shape Casting
// -------------------------

/**
 * @brief Ray-triangle intersection using Möller-Trumbore algorithm
 * @return true if ray intersects triangle, false otherwise
 */
static bool RayIntersectsTriangle(V3 rayOrigin, V3 rayDir,
                                  V3 v0, V3 v1, V3 v2,
                                  float *outDistance, V3 *outNormal)
{
    const float EPSILON = 0.0000001f;

    // Calculate triangle edges
    V3 edge1 = V3_SUB(v1, v0);
    V3 edge2 = V3_SUB(v2, v0);

    // Begin calculating determinant - also used to calculate U parameter
    V3 h = V3_CROSS(rayDir, edge2);
    float a = V3_DOT(edge1, h);

    // Ray is parallel to triangle
    if (fabsf(a) < EPSILON)
        return false;

    float f = 1.0f / a;
    V3 s = V3_SUB(rayOrigin, v0);
    float u = f * V3_DOT(s, h);

    // Intersection is outside triangle
    if (u < 0.0f || u > 1.0f)
        return false;

    V3 q = V3_CROSS(s, edge1);
    float v = f * V3_DOT(rayDir, q);

    // Intersection is outside triangle
    if (v < 0.0f || u + v > 1.0f)
        return false;

    // Calculate t to find out where intersection point is on line
    float t = f * V3_DOT(edge2, q);

    if (t > EPSILON) // Ray intersection
    {
        *outDistance = t;

        // Calculate normal (counter-clockwise winding)
        V3 normal = V3_CROSS(edge1, edge2);
        *outNormal = V3_NORM(normal);

        return true;
    }

    return false; // Line intersection but not ray
}

/**
 * @brief Test ray against mesh collider with BVH optimization
 */
static bool RaycastMesh(V3 origin, V3 direction, EC_Collider *collider,
                        float maxDistance, RaycastHit *outHit)
{
    Mesh *mesh = collider->data.mesh.mesh;
    Transform *transform = collider->transform;

    // Get world transformation
    V3 worldPos = T_WPos(transform);
    V3 worldScale = T_WSca(transform);
    V3 right = T_Right(transform);
    V3 up = T_Up(transform);
    V3 forward = T_Forward(transform);

    // Check if we have a BVH for fast raycast
    MeshBVH *bvh = collider->data.mesh.bvh;
    if (bvh)
    {
        // Use BVH for fast raycast
        // Update BVH with current transform (for dynamic objects)
        // TODO: Only update if transform changed significantly
        MeshBVH_UpdateTransform(bvh, mesh, worldPos, worldScale, right, up, forward, collider->offset);

        BVHRaycastHit bvhHit;
        if (MeshBVH_Raycast(bvh, origin, direction, maxDistance, &bvhHit))
        {
            outHit->hit = true;
            outHit->collider = collider;
            outHit->distance = bvhHit.distance;
            outHit->point = bvhHit.point;
            outHit->normal = bvhHit.normal;
            return true;
        }
        return false;
    }

    // Fallback to brute force raycast for meshes without BVH
    LogWarning(&_logConfig, "Performing brute force raycast on mesh without BVH - consider building BVH for better performance");

    float closestDistance = maxDistance;
    bool hit = false;
    V3 hitNormal = V3_ZERO;

    // Test against each triangle (original brute force method)
    if (mesh->indices && mesh->indices_size > 0)
    {
        for (size_t i = 0; i < mesh->indices_size; i += 3)
        {
            // Get triangle vertices in local space
            uint32_t idx0 = mesh->indices[i];
            uint32_t idx1 = mesh->indices[i + 1];
            uint32_t idx2 = mesh->indices[i + 2];

            // Get vertex positions
            V3 v0_local = mesh->vertices[idx0].position;
            V3 v1_local = mesh->vertices[idx1].position;
            V3 v2_local = mesh->vertices[idx2].position;

            // Transform vertices to world space
            // Apply scale
            v0_local = V3_MUL(v0_local, worldScale);
            v1_local = V3_MUL(v1_local, worldScale);
            v2_local = V3_MUL(v2_local, worldScale);

            // Apply rotation (using transform basis vectors)
            V3 v0_world = {
                v0_local.x * right.x + v0_local.y * up.x + v0_local.z * forward.x + worldPos.x,
                v0_local.x * right.y + v0_local.y * up.y + v0_local.z * forward.y + worldPos.y,
                v0_local.x * right.z + v0_local.y * up.z + v0_local.z * forward.z + worldPos.z};
            V3 v1_world = {
                v1_local.x * right.x + v1_local.y * up.x + v1_local.z * forward.x + worldPos.x,
                v1_local.x * right.y + v1_local.y * up.y + v1_local.z * forward.y + worldPos.y,
                v1_local.x * right.z + v1_local.y * up.z + v1_local.z * forward.z + worldPos.z};
            V3 v2_world = {
                v2_local.x * right.x + v2_local.y * up.x + v2_local.z * forward.x + worldPos.x,
                v2_local.x * right.y + v2_local.y * up.y + v2_local.z * forward.y + worldPos.y,
                v2_local.x * right.z + v2_local.y * up.z + v2_local.z * forward.z + worldPos.z};

            // Apply collider offset
            V3 offsetWorld = {
                collider->offset.x * right.x + collider->offset.y * up.x + collider->offset.z * forward.x,
                collider->offset.x * right.y + collider->offset.y * up.y + collider->offset.z * forward.y,
                collider->offset.x * right.z + collider->offset.y * up.z + collider->offset.z * forward.z};
            v0_world = V3_ADD(v0_world, offsetWorld);
            v1_world = V3_ADD(v1_world, offsetWorld);
            v2_world = V3_ADD(v2_world, offsetWorld);

            // Test ray against triangle
            float distance;
            V3 normal;
            if (RayIntersectsTriangle(origin, direction, v0_world, v1_world, v2_world,
                                      &distance, &normal))
            {
                if (distance < closestDistance)
                {
                    closestDistance = distance;
                    hitNormal = normal;
                    hit = true;
                }
            }
        }
    }
    else if (mesh->vertices && mesh->vertices_size > 0)
    {
        // Non-indexed mesh - iterate vertices by 3
        for (size_t i = 0; i < mesh->vertices_size; i += 3)
        {
            V3 v0_local = mesh->vertices[i].position;
            V3 v1_local = mesh->vertices[i + 1].position;
            V3 v2_local = mesh->vertices[i + 2].position;

            // Apply scale
            v0_local = V3_MUL(v0_local, worldScale);
            v1_local = V3_MUL(v1_local, worldScale);
            v2_local = V3_MUL(v2_local, worldScale);

            // Apply rotation
            V3 v0_world = {
                v0_local.x * right.x + v0_local.y * up.x + v0_local.z * forward.x + worldPos.x,
                v0_local.x * right.y + v0_local.y * up.y + v0_local.z * forward.y + worldPos.y,
                v0_local.x * right.z + v0_local.y * up.z + v0_local.z * forward.z + worldPos.z};
            V3 v1_world = {
                v1_local.x * right.x + v1_local.y * up.x + v1_local.z * forward.x + worldPos.x,
                v1_local.x * right.y + v1_local.y * up.y + v1_local.z * forward.y + worldPos.y,
                v1_local.x * right.z + v1_local.y * up.z + v1_local.z * forward.z + worldPos.z};
            V3 v2_world = {
                v2_local.x * right.x + v2_local.y * up.x + v2_local.z * forward.x + worldPos.x,
                v2_local.x * right.y + v2_local.y * up.y + v2_local.z * forward.y + worldPos.y,
                v2_local.x * right.z + v2_local.y * up.z + v2_local.z * forward.z + worldPos.z};

            // Apply collider offset
            V3 offsetWorld = {
                collider->offset.x * right.x + collider->offset.y * up.x + collider->offset.z * forward.x,
                collider->offset.x * right.y + collider->offset.y * up.y + collider->offset.z * forward.y,
                collider->offset.x * right.z + collider->offset.y * up.z + collider->offset.z * forward.z};
            v0_world = V3_ADD(v0_world, offsetWorld);
            v1_world = V3_ADD(v1_world, offsetWorld);
            v2_world = V3_ADD(v2_world, offsetWorld);

            float distance;
            V3 normal;
            if (RayIntersectsTriangle(origin, direction, v0_world, v1_world, v2_world,
                                      &distance, &normal))
            {
                if (distance < closestDistance)
                {
                    closestDistance = distance;
                    hitNormal = normal;
                    hit = true;
                }
            }
        }
    }

    if (hit)
    {
        outHit->hit = true;
        outHit->collider = collider;
        outHit->distance = closestDistance;
        outHit->point = V3_ADD(origin, V3_SCALE(direction, closestDistance));
        outHit->normal = hitNormal;
        return true;
    }

    return false;
}

bool PhysicsManager_Raycast(V3 origin, V3 direction, float maxDistance, RaycastHit *outHit, uint32_t layerMask)
{
    if (!_manager || !outHit)
        return false;

    direction = V3_NORM(direction);

    RaycastHit closestHit = {0};
    closestHit.distance = maxDistance;
    closestHit.hit = false;

    // Check all colliders
    for (int i = 0; i < _manager->rigidbodies_size; i++)
    {
        if (!_manager->rigidbodies[i])
            continue;
        // Check if the collider is raycastable
        if (!(COLLISION_MASK[_manager->rigidbodies[i]->component->entity->layer] & (1 << E_LAYER_RAYCAST)))
            continue;
        // Check if the collider belongs to the specified layer mask
        if (!(COLLISION_MASK[_manager->rigidbodies[i]->component->entity->layer] & layerMask))
            continue;

        EC_Collider *collider = _manager->rigidbodies[i]->ec_collider;
        if (!collider)
            continue;

        // Quick AABB rejection test
        AABB aabb = collider->worldAABB;

        // Calculate intersection using slab method
        V3 invDir = {
            (fabsf(direction.x) > 0.0001f) ? 1.0f / direction.x : INFINITY,
            (fabsf(direction.y) > 0.0001f) ? 1.0f / direction.y : INFINITY,
            (fabsf(direction.z) > 0.0001f) ? 1.0f / direction.z : INFINITY};

        float t1 = (aabb.min.x - origin.x) * invDir.x;
        float t2 = (aabb.max.x - origin.x) * invDir.x;
        float t3 = (aabb.min.y - origin.y) * invDir.y;
        float t4 = (aabb.max.y - origin.y) * invDir.y;
        float t5 = (aabb.min.z - origin.z) * invDir.z;
        float t6 = (aabb.max.z - origin.z) * invDir.z;

        float tmin = fmaxf(fmaxf(fminf(t1, t2), fminf(t3, t4)), fminf(t5, t6));
        float tmax = fminf(fminf(fmaxf(t1, t2), fmaxf(t3, t4)), fmaxf(t5, t6));

        // Skip if ray doesn't intersect AABB
        if (tmax < tmin || tmax < 0.0f || tmin > closestHit.distance)
            continue;

        // Collision test based on collider type
        RaycastHit tempHit = {0};

        if (collider->type == EC_COLLIDER_MESH)
        {
            // Use precise mesh raycast
            if (RaycastMesh(origin, direction, collider, closestHit.distance, &tempHit))
            {
                if (tempHit.distance < closestHit.distance)
                {
                    closestHit = tempHit;
                }
            }
        }
        else
        {
            // For box/sphere/capsule, AABB test is sufficient (or add precise tests)
            float hitDistance = (tmin > 0.0f) ? tmin : tmax;

            if (hitDistance < closestHit.distance)
            {
                closestHit.hit = true;
                closestHit.collider = collider;
                closestHit.distance = hitDistance;
                closestHit.point = V3_ADD(origin, V3_SCALE(direction, hitDistance));

                // Calculate normal based on which face was hit
                V3 center = V3_SCALE(V3_ADD(aabb.min, aabb.max), 0.5f);
                V3 localHit = V3_SUB(closestHit.point, center);
                V3 size = V3_SCALE(V3_SUB(aabb.max, aabb.min), 0.5f);

                // Determine which axis has the largest normalized component
                V3 normalized = {
                    localHit.x / size.x,
                    localHit.y / size.y,
                    localHit.z / size.z};

                float absX = fabsf(normalized.x);
                float absY = fabsf(normalized.y);
                float absZ = fabsf(normalized.z);

                if (absX > absY && absX > absZ)
                    closestHit.normal = (V3){(normalized.x > 0) ? 1.0f : -1.0f, 0, 0};
                else if (absY > absZ)
                    closestHit.normal = (V3){0, (normalized.y > 0) ? 1.0f : -1.0f, 0};
                else
                    closestHit.normal = (V3){0, 0, (normalized.z > 0) ? 1.0f : -1.0f};
            }
        }
    }

    *outHit = closestHit;
    return closestHit.hit;
}

int PhysicsManager_BoxCast(V3 center, V3 halfExtents, EC_Collider **outColliders, int maxColliders)
{
    if (!_manager || !outColliders)
        return 0;

    AABB boxAABB = {
        .min = V3_SUB(center, halfExtents),
        .max = V3_ADD(center, halfExtents)};

    int count = 0;
    for (int i = 0; i < _manager->rigidbodies_size && count < maxColliders; i++)
    {
        if (!_manager->rigidbodies[i])
            continue;

        EC_Collider *collider = _manager->rigidbodies[i]->ec_collider;
        if (!collider)
            continue;

        if (AABB_Overlap(boxAABB, collider->worldAABB))
        {
            outColliders[count++] = collider;
        }
    }

    return count;
}

int PhysicsManager_SphereCast(V3 center, float radius, EC_Collider **outColliders, int maxColliders)
{
    if (!_manager || !outColliders)
        return 0;

    float radiusSq = radius * radius;
    int count = 0;

    for (int i = 0; i < _manager->rigidbodies_size && count < maxColliders; i++)
    {
        if (!_manager->rigidbodies[i])
            continue;

        EC_Collider *collider = _manager->rigidbodies[i]->ec_collider;
        if (!collider)
            continue;

        // Find closest point on AABB to sphere center
        AABB aabb = collider->worldAABB;
        V3 closest = {
            fmaxf(aabb.min.x, fminf(center.x, aabb.max.x)),
            fmaxf(aabb.min.y, fminf(center.y, aabb.max.y)),
            fmaxf(aabb.min.z, fminf(center.z, aabb.max.z))};

        // Check if closest point is within sphere
        V3 diff = V3_SUB(closest, center);
        float distSq = V3_DOT(diff, diff);

        if (distSq <= radiusSq)
        {
            outColliders[count++] = collider;
        }
    }

    return count;
}

// -------------------------
// Physics Integration
// -------------------------

// -------------------------
// Improved Velocity Integration with Better Sleeping
// -------------------------

static void IntegrateVelocity(EC_RigidBody *rigidbody, float deltaTime)
{
    if (!rigidbody || rigidbody->isKinematic || *rigidbody->isStatic)
        return;

    if (rigidbody->isSleeping)
        return;

    // Apply gravity
    if (rigidbody->useGravity)
    {
        rigidbody->velocity = V3_ADD(rigidbody->velocity, V3_SCALE(_manager->gravity, deltaTime));
    }

    // Apply accumulated forces
    if (rigidbody->mass > 0.0f)
    {
        V3 acceleration = V3_SCALE(rigidbody->forceAccum, 1.0f / rigidbody->mass);
        rigidbody->velocity = V3_ADD(rigidbody->velocity, V3_SCALE(acceleration, deltaTime));
    }

    // Apply linear damping
    if (rigidbody->linearDamping > 0.0f)
    {
        float damping = expf(-rigidbody->linearDamping * deltaTime);
        rigidbody->velocity = V3_SCALE(rigidbody->velocity, damping);
    }

    // Integrate angular forces (torque)
    rigidbody->angularVelocity = V3_ADD(rigidbody->angularVelocity,
                                        V3_SCALE(rigidbody->torqueAccum, deltaTime));

    // Apply angular damping
    if (rigidbody->angularDamping > 0.0f)
    {
        float damping = expf(-rigidbody->angularDamping * deltaTime);
        rigidbody->angularVelocity = V3_SCALE(rigidbody->angularVelocity, damping);
    }

    // Calculate magnitudes for sleeping check
    float velocityMag = V3_MAGNITUDE(rigidbody->velocity);
    float angularMag = V3_MAGNITUDE(rigidbody->angularVelocity);

    // Enhanced sleeping logic
    if (velocityMag < SLEEP_VELOCITY_THRESHOLD && angularMag < SLEEP_ANGULAR_THRESHOLD)
    {
        rigidbody->sleepTimer += deltaTime;

        // Apply aggressive damping near sleep threshold
        if (rigidbody->sleepTimer > SLEEP_TIME_THRESHOLD * 0.5f)
        {
            rigidbody->velocity = V3_SCALE(rigidbody->velocity, RESTING_VELOCITY_DAMPING);
            rigidbody->angularVelocity = V3_SCALE(rigidbody->angularVelocity, RESTING_VELOCITY_DAMPING);
        }

        if (rigidbody->sleepTimer >= SLEEP_TIME_THRESHOLD)
        {
            rigidbody->isSleeping = true;
            rigidbody->velocity = V3_ZERO;
            rigidbody->angularVelocity = V3_ZERO;
        }
    }
    else if (velocityMag > WAKE_VELOCITY_THRESHOLD || angularMag > WAKE_VELOCITY_THRESHOLD)
    {
        rigidbody->sleepTimer = 0.0f;
    }

    // Clear accumulators
    rigidbody->forceAccum = V3_ZERO;
    rigidbody->torqueAccum = V3_ZERO;
}

// -------------------------
// Position Integration WITH ROTATION
// -------------------------

static void IntegratePosition(EC_RigidBody *rigidbody, float deltaTime)
{
    if (!rigidbody || rigidbody->isKinematic || *rigidbody->isStatic)
        return;

    if (rigidbody->isSleeping)
        return;

    // Apply position constraints
    if (rigidbody->constraints.freezePositionX)
        rigidbody->velocity.x = 0.0f;
    if (rigidbody->constraints.freezePositionY)
        rigidbody->velocity.y = 0.0f;
    if (rigidbody->constraints.freezePositionZ)
        rigidbody->velocity.z = 0.0f;

    // Apply rotation constraints
    if (rigidbody->constraints.freezeRotationX)
        rigidbody->angularVelocity.x = 0.0f;
    if (rigidbody->constraints.freezeRotationY)
        rigidbody->angularVelocity.y = 0.0f;
    if (rigidbody->constraints.freezeRotationZ)
        rigidbody->angularVelocity.z = 0.0f;

    Transform *transform = &rigidbody->component->entity->transform;

    // Integrate linear position
    V3 deltaPos = V3_SCALE(rigidbody->velocity, deltaTime);
    T_LPos_Add(transform, deltaPos);

    // ===== INTEGRATE ROTATION =====
    // Using semi-implicit Euler integration for quaternions
    float angularSpeed = V3_MAGNITUDE(rigidbody->angularVelocity);

    if (angularSpeed > 0.0001f)
    {
        // Get current rotation
        Quaternion currentRot = T_LRot(transform); // Get local rotation quaternion

        // Normalize angular velocity to get axis
        V3 axis = V3_SCALE(rigidbody->angularVelocity, 1.0f / angularSpeed);

        // Calculate rotation angle for this timestep
        float angle = angularSpeed * deltaTime;

        // Create rotation quaternion from axis-angle
        float halfAngle = angle * 0.5f;
        float sinHalfAngle = sinf(halfAngle);

        Quaternion deltaRot;
        deltaRot.x = axis.x * sinHalfAngle;
        deltaRot.y = axis.y * sinHalfAngle;
        deltaRot.z = axis.z * sinHalfAngle;
        deltaRot.w = cosf(halfAngle);

        // Apply rotation: newRot = deltaRot * currentRot
        Quaternion newRot = Quat_Mul(deltaRot, currentRot);

        // Normalize to prevent drift
        newRot = Quat_Norm(newRot);

        // Set the new rotation
        T_LRot_Set(transform, newRot);

        // Update world rotation cache if you're using it
        rigidbody->w_rot = newRot;
    }
}

// -------------------------
// Functions
// -------------------------

void PhysicsManager_FixedUpdate(PhysicsManager *physicsManager)
{
    // Step 1: Integrate velocities
    for (int i = 0; i < physicsManager->rigidbodies_size; i++)
    {
        if (physicsManager->rigidbodies[i])
        {
            IntegrateVelocity(physicsManager->rigidbodies[i], physicsManager->timeStep);
        }
    }

    // Step 2: Collision detection
    BroadPhase();
    NarrowPhase();

    // Step 3: Integrate positions
    for (int i = 0; i < _manager->rigidbodies_size; i++)
    {
        if (_manager->rigidbodies[i])
        {
            IntegratePosition(_manager->rigidbodies[i], _manager->timeStep);
        }
    }
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
// Creation & Freeing
// -------------------------

PhysicsManager *PhysicsManager_Create(float timeStep)
{
    PhysicsManager *manager = malloc(sizeof(PhysicsManager));
    manager->rigidbodies_size = 0;
    manager->rigidbodies = NULL;

    // Initialize physics settings
    manager->gravity = (V3){0.0f, PHYSICS_GRAVITY_EARTH, 0.0f};
    manager->timeStep = timeStep;

    LogCreate(&_logConfig, "");
    PhysicsManager_Select(manager);
    return manager;
}

void PhysicsManager_Free(PhysicsManager *manager)
{
    PhysicsManager_Select(manager);
    // Rigidbodies
    free(manager->rigidbodies);
    free(manager);
    LogFree(&_logConfig, "");
}