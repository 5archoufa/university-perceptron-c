#include "entity/components/ec_collider/ec_collider.h"
// Entity
#include "entity/entity.h"
#include "entity/transform.h"
// AABB
#include "physics/aabb.h"
// Mesh
#include "rendering/mesh/mesh.h"
// Math
#include "utilities/math/v3.h"
// C
#include <stdlib.h>
#include <math.h>

// -------------------------
// Static Variables
// -------------------------

static LogConfig _logConfig = {"EC_Collider", LOG_LEVEL_INFO, LOG_COLOR_BLUE};

// -------------------------
// Creation & Freeing
// -------------------------

void EC_Collider_Free(Component *component)
{
    EC_Collider *ec_collider = component->self;
#ifdef DEBUG_COLLIDERS
    Mesh_MarkUnreferenced(ec_collider->debugMesh);
#endif
    // Free collision tracking arrays
    if (ec_collider->currentCollisions)
    {
        free(ec_collider->currentCollisions);
    }
    free(ec_collider);
}

EC_Collider *EC_Collider_CreateBox(EC_Collider *ec_collider, V3 scale)
{
    ec_collider->data.box.scale = scale;
    // ============ Pre-compute Local AABB ============ //
    V3 halfBox = V3_SCALE(scale, 0.5f);
    ec_collider->localAABB.min.x = -halfBox.x + ec_collider->offset.x;
    ec_collider->localAABB.min.y = -halfBox.y + ec_collider->offset.y;
    ec_collider->localAABB.min.z = -halfBox.z + ec_collider->offset.z;
    ec_collider->localAABB.max.x = halfBox.x + ec_collider->offset.x;
    ec_collider->localAABB.max.y = halfBox.y + ec_collider->offset.y;
    ec_collider->localAABB.max.z = halfBox.z + ec_collider->offset.z;
    // ============ Compute World AABB (If Static) ============ //
    Transform *transform = ec_collider->transform;
    if (*ec_collider->isStatic)
    {
        V3 w_pos = T_WPos(transform);
        V3 w_scale = T_WSca(transform);
        V3 right = T_Right(transform);
        V3 up = T_Up(transform);
        V3 forward = T_Forward(transform);

        // Compute local center and half-size (localAABB already includes offset)
        V3 localCenter = V3_SCALE(V3_ADD(ec_collider->localAABB.min, ec_collider->localAABB.max), 0.5f);
        V3 localHalf = V3_SCALE(V3_SUB(ec_collider->localAABB.max, ec_collider->localAABB.min), 0.5f);

        // Apply entity scale
        localCenter = V3_MUL(localCenter, w_scale);
        localHalf = V3_ABS(V3_MUL(localHalf, w_scale)); // ensure positive

        // Compute world-space half-extents (rotation-aware)
        V3 absRight = {fabsf(right.x), fabsf(right.y), fabsf(right.z)};
        V3 absUp = {fabsf(up.x), fabsf(up.y), fabsf(up.z)};
        V3 absForward = {fabsf(forward.x), fabsf(forward.y), fabsf(forward.z)};
        V3 halfWorld = {
            localHalf.x * absRight.x + localHalf.y * absUp.x + localHalf.z * absForward.x,
            localHalf.x * absRight.y + localHalf.y * absUp.y + localHalf.z * absForward.y,
            localHalf.x * absRight.z + localHalf.y * absUp.z + localHalf.z * absForward.z,
        };

        // Final world center (localCenter already includes offset, just add world position)
        V3 worldCenter = V3_ADD(w_pos, localCenter);

        // Construct world-space AABB
        ec_collider->worldAABB.min = V3_SUB(worldCenter, halfWorld);
        ec_collider->worldAABB.max = V3_ADD(worldCenter, halfWorld);
    }

#ifdef DEBUG_COLLIDERS
    // ============ DEBUG:: Create debugging mesh ============ //
    ec_collider->debugColor = 0xff00ff00; // Green wireframe by default
    // Create a wireframe cube mesh for visualization with same pivot as rendered mesh
    ec_collider->debugMesh = Mesh_CreateCubeWireframe(scale, V3_HALF, ec_collider->debugColor);
    Mesh_MarkReferenced(ec_collider->debugMesh);
#endif
    return ec_collider;
}

EC_Collider *EC_Collider_CreateSphere(EC_Collider *ec_collider, float radius)
{
    ec_collider->data.sphere.radius = radius;
    // ============ Pre-compute Local AABB ============ //
    AABB localAABB = {
        {-radius + ec_collider->offset.x,
         -radius + ec_collider->offset.y,
         -radius + ec_collider->offset.z},
        {radius + ec_collider->offset.x,
         radius + ec_collider->offset.y,
         radius + ec_collider->offset.z},
    };
    ec_collider->localAABB = localAABB;
    // ============ Compute World AABB (If Static) ============ //
    Transform *transform = ec_collider->transform;
    if (*ec_collider->isStatic)
    {
        V3 w_pos = T_WPos(transform);
        V3 w_scale = T_WSca(transform);
        float maxScale = fmaxf(fmaxf(w_scale.x, w_scale.y), w_scale.z);
        float worldRadius = radius * maxScale;
        V3 worldCenter = V3_ADD(w_pos, ec_collider->offset);
        V3 radiusVec = (V3){worldRadius, worldRadius, worldRadius};
        AABB worldAABB = {
            V3_SUB(worldCenter, radiusVec),
            V3_ADD(worldCenter, radiusVec),
        };
        ec_collider->worldAABB = worldAABB;
    }

#ifdef DEBUG_COLLIDERS
    // ============ DEBUG:: Create debugging mesh ============ //
    ec_collider->debugColor = 0xff0000ff; // Blue wireframe for spheres
    // Create a wireframe sphere mesh for visualization (centered at origin)
    ec_collider->debugMesh = Mesh_CreateSphereWireframe(radius, V3_HALF, ec_collider->debugColor);
    Mesh_MarkReferenced(ec_collider->debugMesh);
#endif
    return ec_collider;
}
EC_Collider *EC_Collider_CreateCapsule(EC_Collider *ec_collider, float radius, float height)
{
    ec_collider->data.capsule.radius = radius;
    ec_collider->data.capsule.height = height;
    // ============ Pre-compute Local AABB ============ //
    float r = radius;
    float h = height * 0.5f;
    ec_collider->localAABB.min = (V3){
        -r + ec_collider->offset.x,
        -h - r + ec_collider->offset.y,
        -r + ec_collider->offset.z};
    ec_collider->localAABB.max = (V3){
        r + ec_collider->offset.x,
        h + r + ec_collider->offset.y,
        r + ec_collider->offset.z};
    // ============ Compute World AABB (If Static) ============ //
    Transform *transform = ec_collider->transform;
    if (*ec_collider->isStatic)
    {
        V3 w_pos = T_WPos(transform);
        V3 w_scale = T_WSca(transform);

        // Handle non-uniform scaling
        float radiusScale = (w_scale.x + w_scale.z) * 0.5f;
        float heightScale = w_scale.y;

        float worldRadius = radius * radiusScale;
        float worldHalfHeight = (height * 0.5f) * heightScale;

        // Apply offset in world space
        V3 right = T_Right(transform);
        V3 up = T_Up(transform);
        V3 forward = T_Forward(transform);
        V3 worldOffset = {
            ec_collider->offset.x * right.x + ec_collider->offset.y * up.x + ec_collider->offset.z * forward.x,
            ec_collider->offset.x * right.y + ec_collider->offset.y * up.y + ec_collider->offset.z * forward.y,
            ec_collider->offset.x * right.z + ec_collider->offset.y * up.z + ec_collider->offset.z * forward.z};
        V3 worldCenter = V3_ADD(w_pos, worldOffset);

        ec_collider->worldAABB.min = (V3){
            worldCenter.x - worldRadius,
            worldCenter.y - worldHalfHeight - worldRadius,
            worldCenter.z - worldRadius};
        ec_collider->worldAABB.max = (V3){
            worldCenter.x + worldRadius,
            worldCenter.y + worldHalfHeight + worldRadius,
            worldCenter.z + worldRadius};
    }

#ifdef DEBUG_COLLIDERS
    // ============ DEBUG:: Create debugging mesh ============ //
    ec_collider->debugColor = 0xffff0000; // Red wireframe for capsules
    // Create a wireframe capsule mesh for visualization (centered at origin)
    ec_collider->debugMesh = Mesh_CreateCapsuleWireframe(radius, height, V3_HALF, ec_collider->debugColor);
    Mesh_MarkReferenced(ec_collider->debugMesh);
#endif
    return ec_collider;
}

static EC_Collider *EC_Collider_CreateMesh(EC_Collider *ec_collider, Mesh *mesh)
{
    ec_collider->type = EC_COLLIDER_MESH;
    // ============ Pre-compute Local AABB ============ //
    AABB localAABB;
    Vertex_MinMax(mesh->vertices_size, mesh->vertices, &localAABB.min, &localAABB.max);
    ec_collider->localAABB = localAABB;
    // ============ Compute World AABB (If Static) ============ //
    if (*ec_collider->isStatic)
    {
        Transform *transform = ec_collider->transform;
        V3 position = T_WPos(transform);
        V3 scale = T_WSca(transform);
        V3 right = T_Right(transform);
        V3 up = T_Up(transform);
        V3 forward = T_Forward(transform);

        // Compute local center and half-size
        V3 localCenter = V3_SCALE(V3_ADD(localAABB.min, localAABB.max), 0.5f);
        V3 localHalf = V3_SCALE(V3_SUB(localAABB.max, localAABB.min), 0.5f);

        // Apply entity scale
        localCenter = V3_MUL(localCenter, scale);
        localHalf = V3_ABS(V3_MUL(localHalf, scale)); // ensure positive

        // Rotate offset into world space
        V3 worldOffset = {
            ec_collider->offset.x * right.x + ec_collider->offset.y * up.x + ec_collider->offset.z * forward.x,
            ec_collider->offset.x * right.y + ec_collider->offset.y * up.y + ec_collider->offset.z * forward.y,
            ec_collider->offset.x * right.z + ec_collider->offset.y * up.z + ec_collider->offset.z * forward.z};

        // Compute world-space half-extents (rotation-aware)
        V3 absRight = {fabsf(right.x), fabsf(right.y), fabsf(right.z)};
        V3 absUp = {fabsf(up.x), fabsf(up.y), fabsf(up.z)};
        V3 absForward = {fabsf(forward.x), fabsf(forward.y), fabsf(forward.z)};
        V3 halfWorld = {
            localHalf.x * absRight.x + localHalf.y * absUp.x + localHalf.z * absForward.x,
            localHalf.x * absRight.y + localHalf.y * absUp.y + localHalf.z * absForward.y,
            localHalf.x * absRight.z + localHalf.y * absUp.z + localHalf.z * absForward.z,
        };

        // Final world center
        V3 worldCenter = V3_ADD(V3_ADD(position, worldOffset), localCenter);

        // Construct world-space AABB
        ec_collider->worldAABB.min = V3_SUB(worldCenter, halfWorld);
        ec_collider->worldAABB.max = V3_ADD(worldCenter, halfWorld);
    }

#ifdef DEBUG_COLLIDERS
    // ============ DEBUG:: Create debugging mesh ============ //
    ec_collider->debugColor = 0xffffff00;
    ec_collider->debugMesh = mesh; // Use the provided mesh directly
    LogWarning(&_logConfig, "EC_Collider_CreateMesh: Using provided mesh for debug visualization. Ensure it's a wireframe mesh.");
    Mesh_MarkReferenced(ec_collider->debugMesh);
#endif
    return ec_collider;
}

EC_Collider *EC_Collider_Create(Entity *entity, V3 offset, bool isTrigger, EC_Collider_T type, ColliderData data)
{
    EC_Collider *ec_collider = malloc(sizeof(EC_Collider));

    // Allocate memory for isStatic bool and set value
    ec_collider->isStatic = &entity->isStatic;

    ec_collider->transform = &entity->transform;
    ec_collider->offset = offset;
    ec_collider->isTrigger = isTrigger;
    ec_collider->type = type;
    ec_collider->data = data;

    // Initialize collision event callbacks to NULL
    ec_collider->OnCollisionEnter = NULL;
    ec_collider->OnCollisionStay = NULL;
    ec_collider->OnCollisionExit = NULL;

    // Initialize collision tracking arrays
    ec_collider->currentCollisions = NULL;
    ec_collider->currentCollisions_size = 0;
    ec_collider->currentCollisions_capacity = 0;

    switch (type)
    {
    case EC_COLLIDER_BOX:
        EC_Collider_CreateBox(ec_collider, data.box.scale);
        break;
    case EC_COLLIDER_SPHERE:
        EC_Collider_CreateSphere(ec_collider, data.sphere.radius);
        break;
    case EC_COLLIDER_CAPSULE:
        EC_Collider_CreateCapsule(ec_collider, data.capsule.radius, data.capsule.height);
        break;
    case EC_COLLIDER_MESH:
        EC_Collider_CreateMesh(ec_collider, data.mesh.mesh);
        break;
    default:
        break;
    }
    // Component
    ec_collider->component = Component_Create(ec_collider, entity, EC_T_COLLIDER, EC_Collider_Free, NULL, NULL, NULL, NULL, NULL);
    return ec_collider;
}