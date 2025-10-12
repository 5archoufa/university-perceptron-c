#ifndef EC_COLLIDER_H
#define EC_COLLIDER_H

// Entity
#include "entity/entity.h"
#include "entity/transform.h"
// AABB
#include "physics/aabb.h"
// Mesh
#include "rendering/mesh/mesh.h"

// -------------------------
// Types
// -------------------------

typedef struct
{
    V3 scale;
} BoxCollider;

typedef struct
{
    float radius;
} SphereCollider;

typedef struct
{
    float radius;
    float height;
} CapsuleCollider;

typedef struct
{
    Mesh *mesh;
} MeshCollider;

typedef union
{
    BoxCollider box;
    SphereCollider sphere;
    CapsuleCollider capsule;
    MeshCollider mesh;
} ColliderData;

typedef enum
{
    EC_COLLIDER_BOX,
    EC_COLLIDER_SPHERE,
    EC_COLLIDER_CAPSULE,
    EC_COLLIDER_MESH
} EC_Collider_T;

typedef struct EC_Collider
{
    Component *component;
    // Type
    EC_Collider_T type;
    // Data
    ColliderData data;
    // References
    Transform *transform;
    // Settings
    bool isTrigger;
    V3 offset;
    bool *isStatic;
    AABB localAABB;
    AABB worldAABB;

#ifdef DEBUG_COLLIDERS
    // Debug-only data for visualization
    Mesh *debugMesh; // optional prebuilt wireframe
    uint32_t debugColor;
#endif
} EC_Collider;

// -------------------------
// Creation
// -------------------------

EC_Collider *EC_Collider_Create(Entity *entity, V3 offset, bool isTrigger, bool isStatic, EC_Collider_T type, ColliderData data);

#endif