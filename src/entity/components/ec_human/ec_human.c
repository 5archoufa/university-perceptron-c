#include "entity/components/ec_human/ec_human.h"
// Entity
#include "entity/entity.h"
// Shader
#include "rendering/shader/shader-manager.h"
#include "rendering/shader/shader.h"
// Material
#include "rendering/material/material.h"
// Mesh
#include "rendering/mesh/mesh.h"
// Renderer3D
#include "entity/components/ec_renderer3d/ec_renderer3d.h"
// C
#include <stdlib.h>

static const uint32_t COLOR_HUMAN_SKIN_WHITE = 0xffbde0ff; // #ffe0bdff

static void EC_Human_Free(Component *component)
{
    EC_Human *ec_human = component->self;
    free(ec_human);
}

EC_Human *EC_Human_Create(Entity *entity)
{
    EC_Human *ec_human = malloc(sizeof(EC_Human));
    // Renderer
    Mesh* mesh = Mesh_CreateCube((V3){0.5f,1.8f,0.5f}, (V3){0.5f, 0.0f, 0.5f}, COLOR_HUMAN_SKIN_WHITE);
    Shader* shader = ShaderManager_Get(SHADER_TOON_SOLID);
    Material *material = Material_Create(shader, 0, NULL);
    EC_Renderer3D *ec_renderer3d = EC_Renderer3D_Create(entity, mesh, material);
    // Creature
    ec_human->creature = EC_Creature_Create(entity, CREATURE_T_HUMAN, ec_renderer3d, (V2){1.0f, 1.0f}, (V2){1.0f, 1.0f});
    // Component
    ec_human->component = Component_Create(ec_human, entity, EC_T_HUMAN, EC_Human_Free, NULL, NULL, NULL, NULL, NULL);
    return ec_human;
}
