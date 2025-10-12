#include "entity/components/ec_collider/ec_collider.h"
// Entity
#include "entity/entity.h"
#include "entity/transform.h"
// AABB
#include "physics/aabb.h"
// Mesh
#include "rendering/mesh/mesh.h"

// -------------------------
// Creation & Freeing
// -------------------------

void EC_Collider_Free(Component *component)
{
    EC_Collider *ec_collider = component->self;
    free(ec_collider);
}

EC_Collider *EC_Collider_CreateBox(EC_Collider *ec_collider, V3 scale)
{
    ec_collider->data.box.scale = scale;
    // ============ Pre-compute Local AABB ============ //
    V3 halfBox = V3_SCALE(ec_collider->data.box.scale, 0.5f);
    ec_collider->localAABB.min.x = -halfBox.x + ec_collider->offset.x;
    ec_collider->localAABB.min.y = -halfBox.y + ec_collider->offset.y;
    ec_collider->localAABB.min.z = -halfBox.z + ec_collider->offset.z;
    ec_collider->localAABB.max.x = halfBox.x + ec_collider->offset.x;
    ec_collider->localAABB.max.y = halfBox.y + ec_collider->offset.y;
    ec_collider->localAABB.max.z = halfBox.z + ec_collider->offset.z;
    // ============ Compute World AABB (If Static) ============ //

#ifdef DEBUG_COLLIDERS
    // ============ DEBUG:: Create debugging mesh ============ //
    // Mesh* mesh = Mesh_CreateCube();
#endif
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
    if (ec_collider->isStatic)
    {
        V3 w_pos = T_WPos(transform);
        V3 w_scale = T_WSca(transform);
        float maxScale = fmaxf(fmaxf(w_scale.x, w_scale.y), w_scale.z);
        float worldRadius = radius * maxScale;
        V3 worldCenter = V3_ADD(w_pos, ec_collider->offset);
        AABB worldAABB = {
            V3_SUB(worldCenter, (V3){worldRadius, worldRadius, worldRadius}),
            V3_ADD(worldCenter, (V3){worldRadius, worldRadius, worldRadius}),
        };
        ec_collider->worldAABB = worldAABB;
    }
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
    return ec_collider;
}

EC_Collider *EC_Collider_Create(Entity *entity, V3 offset, bool isTrigger, bool isStatic, EC_Collider_T type, ColliderData data)
{
    EC_Collider *ec_collider = malloc(sizeof(EC_Collider));
    *ec_collider->isStatic = isStatic;
    ec_collider->transform = &entity->transform;
    ec_collider->offset = offset;
    ec_collider->isTrigger = isTrigger;
    ec_collider->transform = &entity->transform;
    ec_collider->type = type;
    switch (type)
    {
    case EC_COLLIDER_BOX:
        EC_Collider_CreateBox(ec_collider, data.box.scale);
    case EC_COLLIDER_SPHERE:
        EC_Collider_CreateSphere(ec_collider, data.sphere.radius);
    case EC_COLLIDER_CAPSULE:
        EC_Collider_CreateCapsule(ec_collider, data.capsule.radius, data.capsule.height);
    case EC_COLLIDER_MESH:
        EC_Collider_CreateMesh(ec_collider, data.mesh.mesh);
    default:
    }
    // Component
    ec_collider->component = Component_Create(ec_collider, entity, EC_T_COLLIDER, EC_Collider_Free, NULL, NULL, NULL, NULL, NULL);
    return ec_collider;
}