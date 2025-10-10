#include "entity/components/ec_renderer3d/ec_renderer3d.h"
#include "perceptron.h"
#include "entity/entity.h"
#include "rendering/mesh/mesh.h"
#include "rendering/texture/texture.h"
#include <cglm/cglm.h>
#include <stdint.h>
#include "entity/transform.h"

static Material *_defaultMaterial = NULL;
static LogConfig _logConfig = {"EC_Renderer3D", LOG_LEVEL_INFO, LOG_COLOR_BLUE};

// -------------------------
// Entity Events
// -------------------------

void EC_Renderer3D_Update(Component *component)
{
    // component->entity->eulerAngles.y += 0.1f * DeltaTime;
    // component->entity->eulerAngles.x += 0.05f * DeltaTime;
    // component->entity->eulerAngles.z += 0.08f * DeltaTime;
    EC_Renderer3D *renderer = component->self;
    // component->entity->scale.x = 1.0 + 0.3 * sinf(0.001f * PerceptronTime * DeltaTime);
    // component->entity->scale.y = 1.0 + 0.3 * sinf(0.001f * PerceptronTime * DeltaTime + 2.0f);
    // component->entity->scale.z = 1.0 + 0.3 * sinf(0.001f * PerceptronTime * DeltaTime + 4.0f);
}

// -------------------------
// Creation and Freeing
// -------------------------

static void EC_Renderer3D_Free(Component *component)
{
    EC_Renderer3D *ec_renderer3d = component->self;
    if (ec_renderer3d->mesh == NULL)
    {
        printf("Warning: EC_Renderer3D_Free: mesh is NULL\n");
    }
    // Mark unreferenced
    Material_MarkUnreferenced(ec_renderer3d->material);
    Mesh_MarkUnreferenced(ec_renderer3d->mesh);
    World_Renderer3D_Remove(ec_renderer3d);
    free(ec_renderer3d);
}

/// @brief
/// @param entity
/// @param mesh
/// @param material If NULL, default material will be used.
/// @return
EC_Renderer3D *EC_Renderer3D_Create(Entity *entity, Mesh *mesh, Material *material)
{
    EC_Renderer3D *ec_renderer3d = malloc(sizeof(EC_Renderer3D));
    // Mesh
    ec_renderer3d->mesh = mesh;
    Mesh_MarkReferenced(mesh);
    // Material
    ec_renderer3d->material = material == NULL ? _defaultMaterial : material;
    Material_MarkReferenced(ec_renderer3d->material);
    // Update bounds
    EC_Renderer3D_CalculateBounds(ec_renderer3d);
    // Component
    ec_renderer3d->component = Component_Create(ec_renderer3d, entity, EC_T_RENDERER3D, EC_Renderer3D_Free, NULL, NULL, EC_Renderer3D_Update, NULL, NULL);
    World_Renderer3D_Add(ec_renderer3d);
    return ec_renderer3d;
}

// -------------------------
// EC_Renderer3D Functions
// -------------------------

void EC_Renderer3D_SetDefaultMaterial(Material *material)
{
    _defaultMaterial = material;
    Material_MarkReferenced(material);
    Log(&_logConfig, "Default Material set to %s", material->shader->name);
}

void EC_Renderer3D_CalculateBounds(EC_Renderer3D *ec_renderer3d)
{
    V3 min = {FLT_MAX, FLT_MAX, FLT_MAX};
    V3 max = {FLT_MIN, FLT_MIN, FLT_MIN};
    for (int i = 0; i < ec_renderer3d->mesh->vertex_count; i++)
    {
        Vertex vertex = ec_renderer3d->mesh->vertices[i];
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
    ec_renderer3d->bounds.start = min;
    ec_renderer3d->bounds.end = max;
    ec_renderer3d->bounds.size = (V3){max.x - min.x, max.y - min.y, max.z - min.z};
}

// -------------------------
// Prefabs
// -------------------------

EC_Renderer3D *Prefab_Cube(Entity *parent, TransformSpace TS, V3 position, Quaternion rotation, V3 scale, float meshSize, Texture *texture)
{
    Entity *entity = Entity_Create(parent, "Cube", TS, position, rotation, scale);
    Mesh *cubeMesh = Mesh_CreateCube((V3){meshSize, meshSize, meshSize}, V3_HALF, 0xffffffff);
    return EC_Renderer3D_Create(entity, cubeMesh, NULL);
}