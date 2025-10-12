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

// -------------------------
// Types
// -------------------------

typedef struct Proxy{
    uint32_t id;
    AABB aabb;
    Entity *entity;
    // Colision Mask?
    bool isStatic;
} Proxy;

typedef struct PhysicsManager
{
    size_t rigid_bodies_size;
    EC_RigidBody **rigid_bodies;
    bool **_elayerFilter;
    size_t _elayers_Size;
    Proxy *proxies;
} PhysicsManager;

// -------------------------
// Manager Management
// -------------------------

void PhysicsManager_Select(PhysicsManager *physicsManager);

// -------------------------
// Creation & Freeing
// -------------------------

PhysicsManager *PhysicsManager_Create();
void PhysicsManager_Free(PhysicsManager *manager);

#endif