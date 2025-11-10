#include "entity/components/ec_rabbit/ec_rabbit.h"
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



// ============ Yowyoh ============ //

static const uint32_t COLOR_RABBIT_SKIN_BROWN = 0xff3281c6; // Brown fur color

static void EC_Rabbit_Free(Component *component)
{
    EC_Rabbit *ec_rabbit = component->self;
    free(ec_rabbit);
}

EC_Rabbit *EC_Rabbit_Create(EC_Island *ec_island, Entity *e_rabbit)
{
    EC_Rabbit *ec_rabbit = malloc(sizeof(EC_Rabbit));
    
    // ============ Body ============ //
    Entity *e_body = Entity_Create(e_rabbit, false, "Body", TS_LOCAL, (V3){0, 0, 0}, QUATERNION_IDENTITY, V3_ONE);
    // Mesh - smaller, rounder body for rabbit
    Mesh* mesh = Mesh_CreateCylinder(0.3, 0.6, 6, (V3){0.5f, 0.0f, 0.5f}, COLOR_RABBIT_SKIN_BROWN);
    V3 meshScale = {0.5f, 0.6f, 0.5f};
    // Renderer
    EC_MeshRenderer *ec_meshRenderer = EC_MeshRenderer_Create(e_body, mesh, meshScale, NULL);
    
    // ============ Head ============ //
    float headDiameter = 0.5; // Smaller head for rabbit
    // Entity
    Entity *e_head = Entity_Create(e_body, false, "Head", TS_LOCAL, (V3){0, 0.6f - (headDiameter * 0.5f), 0}, QUATERNION_IDENTITY, V3_ONE);
    // Mesh
    Mesh *mesh_head = Mesh_CreateSphere(headDiameter, 6, 10, (V3){0.5, 0, 0.5}, COLOR_RABBIT_SKIN_BROWN, false);
    V3 meshScale_head = {headDiameter, headDiameter, headDiameter};
    // Renderer
    EC_MeshRenderer *ec_meshRenderer_head = EC_MeshRenderer_Create(e_head, mesh_head, meshScale_head, NULL);
    
    // ============ Eyes ============ //
    float eyeDiameter = 0.1f; // Smaller eyes for rabbit
    float eyeOffsetX = 0.15f;
    float eyeOffsetY = 0.15f;
    float eyeOffsetZ = headDiameter * 0.5f;
    Mesh *mesh_eye = Mesh_CreateSphere(eyeDiameter, 4, 6, (V3){0.5, 0.5, 0.5}, 0x000000, false);
    V3 meshScale_eye = {eyeDiameter, eyeDiameter, eyeDiameter};
    // Left Eye
    Entity *e_eye_left = Entity_Create(e_head, false, "Eye_Left", TS_LOCAL, (V3){-eyeOffsetX, eyeOffsetY, eyeOffsetZ}, QUATERNION_IDENTITY, V3_ONE);
    EC_MeshRenderer *ec_meshRenderer_eye_left = EC_MeshRenderer_Create(e_eye_left, mesh_eye, meshScale_eye, NULL);
    // Right Eye
    Entity *e_eye_right = Entity_Create(e_head, false, "Eye_Right", TS_LOCAL, (V3){eyeOffsetX, eyeOffsetY, eyeOffsetZ}, QUATERNION_IDENTITY, V3_ONE);
    EC_MeshRenderer *ec_meshRenderer_eye_right = EC_MeshRenderer_Create(e_eye_right, mesh_eye, meshScale_eye, NULL);
    
    // ============ Ears ============ //
    float earWidth = 0.1f;
    float earHeight = 0.4f; // Long rabbit ears
    float earOffsetX = 0.15f;
    float earOffsetY = headDiameter * 0.4f;
    Mesh *mesh_ear = Mesh_CreateCylinder(earWidth, earHeight, 6, (V3){0.5f, 0.0f, 0.5f}, COLOR_RABBIT_SKIN_BROWN);
    V3 meshScale_ear = {earWidth, earHeight, earWidth};
    // Left Ear
    Entity *e_ear_left = Entity_Create(e_head, false, "Ear_Left", TS_LOCAL, (V3){-earOffsetX, earOffsetY, 0}, QUATERNION_IDENTITY, V3_ONE);
    EC_MeshRenderer *ec_meshRenderer_ear_left = EC_MeshRenderer_Create(e_ear_left, mesh_ear, meshScale_ear, NULL);
    // Right Ear
    Entity *e_ear_right = Entity_Create(e_head, false, "Ear_Right", TS_LOCAL, (V3){earOffsetX, earOffsetY, 0}, QUATERNION_IDENTITY, V3_ONE);
    EC_MeshRenderer *ec_meshRenderer_ear_right = EC_MeshRenderer_Create(e_ear_right, mesh_ear, meshScale_ear, NULL);
    
    // ============ Creature ============ // 
    ec_rabbit->creature = EC_Creature_Create(ec_island, e_rabbit, CREATURE_T_ANIMAL, ec_meshRenderer, (V2){4.0f, 4.0f}, (V2){100.0f, 100.0f});
    
    // ============ Component ============ // 
    ec_rabbit->component = Component_Create(ec_rabbit, e_rabbit, EC_T_CREATURE, EC_Rabbit_Free, NULL, NULL, NULL, NULL, NULL);
    
    // Layer
    Entity_SetLayer(e_body, E_LAYER_CREATURE, true);
    
    return ec_rabbit;
}