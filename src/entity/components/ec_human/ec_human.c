#include "entity/components/ec_human/ec_human.h"
// Perceptron
#include "perceptron.h"
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
#include "entity/components/ec_mesh_renderer/ec_mesh_renderer.h"
// C
#include <stdlib.h>

static const uint32_t COLOR_HUMAN_SKIN_WHITE = 0xffbde0ff; // #ffe0bdff

// -------------------------
// Entity Events
// -------------------------

static void EC_Human_Free(Component *component)
{
    EC_Human *ec_human = component->self;
    free(ec_human);
}

EC_Human *EC_Human_Create(EC_Island *ec_island, Entity *e_human)
{
    EC_Human *ec_human = malloc(sizeof(EC_Human));
    // ============ Body ============ //
    Entity *e_body = Entity_Create(e_human, false, "Body", TS_LOCAL, (V3){0, 0, 0}, QUATERNION_IDENTITY, V3_ONE);
    // Mesh
    Mesh *mesh = Mesh_CreateCylinder(0.2, 1.8, 6, (V3){0.5f, 0.0f, 0.5f}, COLOR_HUMAN_SKIN_WHITE);
    V3 meshScale = {0.4f, 1.8f, 0.4f};
    // Renderer
    EC_MeshRenderer *ec_meshRenderer = EC_MeshRenderer_Create(e_body, mesh, meshScale, NULL);
    // ============ Head ============ //
    float headDiameter = 1;
    // Entity
    Entity *e_head = Entity_Create(e_body, false, "Head", TS_LOCAL, (V3){0, 1.8f - (headDiameter * 0.5f), 0}, QUATERNION_IDENTITY, V3_ONE);
    // Mesh
    Mesh *mesh_head = Mesh_CreateSphere(headDiameter, 6, 10, (V3){0.5, 0, 0.5}, COLOR_HUMAN_SKIN_WHITE, false);
    V3 meshScale_head = {headDiameter, headDiameter, headDiameter};
    // Renderer
    EC_MeshRenderer *ec_meshRenderer_head = EC_MeshRenderer_Create(e_head, mesh_head, meshScale_head, NULL);
    // ============ Eyes ============ //
    float eyeDiameter = 0.2f;
    float eyeOffsetX = 0.2f;
    float eyeOffsetY = 0.5f;
    float eyeOffsetZ = headDiameter * 0.5f;
    Mesh *mesh_eye = Mesh_CreateSphere(eyeDiameter, 4, 6, (V3){0.5, 0.5, 0.5}, 0x000000, false);
    V3 meshScale_eye = {eyeDiameter, eyeDiameter, eyeDiameter};
    // Left Eye
    Entity *e_eye_left = Entity_Create(e_head, false, "Eye_Left", TS_LOCAL, (V3){-eyeOffsetX, eyeOffsetY, eyeOffsetZ}, QUATERNION_IDENTITY, V3_ONE);
    EC_MeshRenderer *ec_meshRenderer_eye_left = EC_MeshRenderer_Create(e_eye_left, mesh_eye, meshScale_eye, NULL);
    // Right Eye
    Entity *e_eye_right = Entity_Create(e_head, false, "Eye_Right", TS_LOCAL, (V3){eyeOffsetX, eyeOffsetY, eyeOffsetZ}, QUATERNION_IDENTITY, V3_ONE);
    EC_MeshRenderer *ec_meshRenderer_eye_right = EC_MeshRenderer_Create(e_eye_right, mesh_eye, meshScale_eye, NULL);
    // ============ Creature ============ //
    ec_human->creature = EC_Creature_Create(ec_island, e_human, CREATURE_T_HUMAN, ec_meshRenderer, (V2){3.0f, 3.0f}, (V2){150.0f, 150.0f});
    // ============ Component ============ //
    ec_human->component = Component_Create(ec_human, e_human, EC_T_HUMAN, EC_Human_Free, NULL, NULL, NULL, NULL, NULL);
    // Layer
    Entity_SetLayer(e_body, E_LAYER_CREATURE, true);
    return ec_human;
}

// ----------------------------------------
// Prefabs
// ----------------------------------------

EC_Human Prefab_Human(Entity *parent, EC_Island *ec_island, TransformSpace TS, V3 position, Quaternion rotation, V3 scale)
{
    // ============ Entity ============ //
    Entity *e_human = Entity_Create(parent, false, "Player", TS, position, rotation, scale);
    // ============ Human ============ //
    EC_Human *ec_human = EC_Human_Create(ec_island, e_human);
    Entity_SetLayer(e_human, E_LAYER_CREATURE, true);
    return *ec_human;
}