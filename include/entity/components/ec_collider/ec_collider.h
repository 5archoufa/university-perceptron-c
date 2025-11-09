#ifndef EC_COLLIDER_H
#define EC_COLLIDER_H

// Entity
#include "entity/entity.h"
#include "entity/transform.h"
// AABB
#include "physics/aabb.h"
// Mesh
#include "rendering/mesh/mesh.h"

// Forward declarations
typedef struct CollisionInfo CollisionInfo;
typedef struct EC_Collider EC_Collider;
typedef struct MeshBVH MeshBVH;
typedef void (*OnCollisionCallback)(EC_Collider *self, CollisionInfo *info);

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
    // Cached BVH pointer for fast access (points to mesh->bvh)
    // This avoids dereferencing mesh->bvh repeatedly during raycasts
    struct MeshBVH *bvh;
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

/**
 * @brief Collider component for physics collision detection
 */
struct EC_Collider
{
    Component *component;
    // Type
    EC_Collider_T type;
    // Data
    ColliderData data;
    // References
    Transform *transform;
    
    // Physics properties
    bool isTrigger;
    /// @brief Local space offset from entity position
    V3 offset;
    /// @brief If true, collider is static (doesn't move)
    bool *isStatic;
    
    // Collision bounds
    AABB localAABB;
    AABB worldAABB;
    
    // -------------------------
    // COLLISION EVENTS
    // -------------------------
    
    /**
     * @brief Called when this collider first touches another collider
     * @note Only called once at the start of the collision
     * @note Set to NULL if you don't need this event
     */
    OnCollisionCallback OnCollisionEnter;
    
    /**
     * @brief Called every physics frame while this collider touches another
     * @note Called repeatedly while collision persists
     * @note Set to NULL if you don't need this event
     */
    OnCollisionCallback OnCollisionStay;
    
    /**
     * @brief Called when this collider stops touching another collider
     * @note Only called once when the collision ends
     * @note Set to NULL if you don't need this event
     */
    OnCollisionCallback OnCollisionExit;
    // Internal collision tracking
    struct EC_Collider **currentCollisions;
    int currentCollisions_size;
    int currentCollisions_capacity;

#ifdef DEBUG_COLLIDERS
    // Debug-only data for visualization
    Mesh *debugMesh; // optional prebuilt wireframe
    uint32_t debugColor;
#endif
};

// -------------------------
// Creation
// -------------------------

EC_Collider *EC_Collider_Create(Entity *entity, V3 offset, bool isTrigger, EC_Collider_T type, ColliderData data);

#endif