#include "entity/components/ec_water/ec_water.h"
// Entity
#include "entity/entity.h"
// Shader
#include "rendering/shader/shader-manager.h"
// Material
#include "rendering/material/material.h"

static void EC_Water_Free(Component *component)
{
    EC_Water *ec_water = component->self;
    free(ec_water);
}

static EC_Water *EC_Water_Create(Entity *entity, EC_Renderer3D *ec_renderer3d_water, Noise *noise)
{
    EC_Water *ec_water = malloc(sizeof(EC_Water));
    ec_water->ec_renderer3d_water = ec_renderer3d_water;
    // Component
    ec_water->component = Component_Create(ec_water, entity, EC_T_WATER, EC_Water_Free, NULL, NULL, NULL, NULL, NULL);
    return ec_water;
}

EC_Water *Prefab_Water(Entity *e_parent, TransformSpace TS, V3 position, Quaternion rotation, V3 scale, V3 meshScale, size_t modifiers_size, NoiseModifier* modifiers)
{
    Entity *e_water = Entity_Create(e_parent, "Water", TS, position, rotation, scale);
    Mesh *mesh = Mesh_CreatePlane((V2){meshScale.x, meshScale.z}, (V2_INT){200, 200}, PIXEL_WHITE, V2_HALF);
    // Renderer3D
    Shader* seaShader = ShaderManager_Get(SHADER_SEA);
    Material* seaMaterial = Material_Create(seaShader, 0, NULL);
    EC_Renderer3D *ec_renderer3d_water = EC_Renderer3D_Create(e_water, mesh, seaMaterial);
    EC_Water *ec_water = EC_Water_Create(e_water, ec_renderer3d_water, NULL);
    return ec_water;
}