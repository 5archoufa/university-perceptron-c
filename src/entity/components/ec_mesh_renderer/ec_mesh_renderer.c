#include "entity/components/ec_mesh_renderer/ec_mesh_renderer.h"
// Perceptron
#include "perceptron.h"
// C
#include <stdint.h>
// Entity
#include "entity/transform.h"
#include "entity/entity.h"
// Mesh
#include "rendering/mesh/mesh-manager.h"
#include "rendering/mesh/mesh.h"
// Texture
#include "rendering/texture/texture.h"
// OpenGL
#define GLFW_INCLUDE_NONE
#include <cglm/cglm.h>

static Material *_defaultMaterial = NULL;
static LogConfig _logConfig = {"EC_MeshRenderer", LOG_LEVEL_INFO, LOG_COLOR_BLUE};

// -------------------------
// Creation and Freeing
// -------------------------

static void EC_MeshRenderer_Free(Component *component)
{
    EC_MeshRenderer *ec_meshRenderer = component->self;
    // Mark unreferenced
    Material_MarkUnreferenced(ec_meshRenderer->material);
    Mesh_MarkUnreferenced(ec_meshRenderer->mesh);
    World_Renderer3D_Remove(ec_meshRenderer);
    free(ec_meshRenderer);
}

/// @brief
/// @param entity
/// @param mesh
/// @param material If NULL, default material will be used.
/// @return
EC_MeshRenderer *EC_MeshRenderer_Create(Entity *entity, Mesh *mesh, V3 meshScale, Material *material)
{
    EC_MeshRenderer *ec_meshRenderer = malloc(sizeof(EC_MeshRenderer));
    // Mesh
    ec_meshRenderer->mesh = mesh;
    ec_meshRenderer->meshScale = meshScale;
    Mesh_MarkReferenced(mesh);
    // Material
    ec_meshRenderer->material = material == NULL ? _defaultMaterial : material;
    Material_MarkReferenced(ec_meshRenderer->material);
    // Update bounds
    EC_MeshRenderer_CalculateBounds(ec_meshRenderer);
    // Component
    ec_meshRenderer->component = Component_Create(ec_meshRenderer, entity, EC_T_RENDERER3D, EC_MeshRenderer_Free, NULL, NULL, NULL, NULL, NULL);
    World_Renderer3D_Add(ec_meshRenderer);
    return ec_meshRenderer;
}

// -------------------------
// EC_MeshRenderer Functions
// -------------------------

void EC_MeshRenderer_SetDefaultMaterial(Material *material)
{
    _defaultMaterial = material;
    Material_MarkReferenced(material);
    Log(&_logConfig, "Default Material set to %s", material->shader->name);
}

void EC_MeshRenderer_CalculateBounds(EC_MeshRenderer *ec_meshRenderer)
{
    V3 min = {FLT_MAX, FLT_MAX, FLT_MAX};
    V3 max = {FLT_MIN, FLT_MIN, FLT_MIN};
    for (int i = 0; i < ec_meshRenderer->mesh->vertices_size; i++)
    {
        Vertex vertex = ec_meshRenderer->mesh->vertices[i];
        if (vertex.position.x < min.x)
            min.x = vertex.position.x;
        if (vertex.position.y < min.y)
            min.y = vertex.position.y;
        if (vertex.position.z < min.z)
            min.z = vertex.position.z;
        if (vertex.position.x > max.x)
            max.x = vertex.position.x;
        if (vertex.position.y > max.y)
            max.y = vertex.position.y;
        if (vertex.position.z > max.z)
            max.z = vertex.position.z;
    }
    ec_meshRenderer->bounds.min = min;
    ec_meshRenderer->bounds.max = max;
    ec_meshRenderer->bounds.size = (V3){max.x - min.x, max.y - min.y, max.z - min.z};
}

// -------------------------
// Prefabs
// -------------------------

EC_MeshRenderer *Prefab_Quad(Entity *parent, bool isStatic, TransformSpace TS, V3 position, Quaternion rotation, V3 scale, V2 meshSize, uint32_t vertexColor, Material* material)
{
    Entity *entity = Entity_Create(parent, isStatic, "Quad", TS, position, rotation, scale);
    Mesh *quadMesh = Mesh_CreateQuad(meshSize, V2_HALF, vertexColor);
    return EC_MeshRenderer_Create(entity, quadMesh, V3_ONE, material);
}

EC_MeshRenderer *Prefab_PlaneDefault(Entity *parent, bool isStatic, TransformSpace TS, V3 position, Quaternion rotation, V3 scale)
{
    Entity *entity = Entity_Create(parent, isStatic, "Plane", TS, position, rotation, scale);
    Mesh *planeMesh = MeshManager_GetDefaultPlane();
    return EC_MeshRenderer_Create(entity, planeMesh, (V3){1.0f, 0.0f, 1.0f}, NULL);
}

EC_MeshRenderer *Prefab_Cube(Entity *parent, bool isStatic, TransformSpace TS, V3 position, Quaternion rotation, V3 scale, V3 meshSize, uint32_t color)
{
    Entity *entity = Entity_Create(parent, isStatic, "Cube", TS, position, rotation, scale);
    Mesh *cubeMesh = Mesh_CreateCube(true, meshSize, V3_HALF, color);
    return EC_MeshRenderer_Create(entity, cubeMesh, meshSize, NULL);
}

EC_MeshRenderer *Prefab_CubeDefault(Entity *parent, bool isStatic, TransformSpace TS, V3 position, Quaternion rotation, V3 scale)
{
    Entity *entity = Entity_Create(parent, isStatic, "Cube", TS, position, rotation, scale);
    Mesh *cubeMesh = MeshManager_GetDefaultCube();
    return EC_MeshRenderer_Create(entity, cubeMesh, V3_ONE, NULL);
}

EC_MeshRenderer *Prefab_Sphere(Entity *parent, bool isStatic, TransformSpace TS, V3 position, Quaternion rotation, V3 scale)
{
    Entity *entity = Entity_Create(parent, isStatic, "Sphere", TS, position, rotation, scale);
    Mesh *sphereMesh = MeshManager_GetDefaultSphere();
    return EC_MeshRenderer_Create(entity, sphereMesh, V3_ONE, NULL);
}

Entity *Prefab_CubeWCollider(Entity *parent, bool isStatic, TransformSpace TS, V3 position, Quaternion rotation, V3 scale, V3 meshSize, uint32_t color)
{
    Entity *entity = Entity_Create(parent, isStatic, "Cube", TS, position, rotation, scale);
    Mesh *cubeMesh = Mesh_CreateCube(true, meshSize, V3_HALF, color);
    EC_MeshRenderer_Create(entity, cubeMesh, meshSize, NULL);
    // Create and attach a collider
    ColliderData colliderData;
    colliderData.box.scale = meshSize;
    EC_Collider *collider = EC_Collider_Create(entity, V3_ZERO, false, EC_COLLIDER_BOX, colliderData);
    RigidBodyConstraints constraints = RigidBodyConstraints_None();
    EC_RigidBody *rigidbody = EC_RigidBody_Create(entity, collider, 1.0f, true, constraints);
    return entity;
}