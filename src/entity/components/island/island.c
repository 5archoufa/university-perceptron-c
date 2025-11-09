#include "entity/components/island/island.h"
#include "entity/components/ec_mesh_renderer/ec_mesh_renderer.h"
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include "logging/logger.h"
#include "entity/transform.h"
// Shaders
#include "rendering/shader/shader.h"
#include "rendering/shader/shader-manager.h"
// Materials
#include "rendering/material/material.h"

static LogConfig _logConfig = {"EC_Island", LOG_LEVEL_INFO, LOG_COLOR_BLUE};

static const int INPUT_INTER_UP = 0;
static const int INPUT_INTER_DOWN = 1;

// Colors must be in ABGR format
static const uint32_t COLOR_GRASS = 0xff288034;    // #348028ff
static const uint32_t COLOR_STONE = 0xff7cdaeb;    // #2a2622ff
static const uint32_t COLOR_SAND = 0xff32963f;     // #3f9632ff

static const size_t ISLAND_LAYERS_SIZE = 3;
static const float ISLAND_Y_MIN = -19;
static const float ISLAND_Y_MAX = 20;
static const NoiseLayer ISLAND_LAYERS[] = {
    {{ISLAND_Y_MIN, -6}, COLOR_STONE},
    {{-6, 0.0}, COLOR_SAND},
    {{0.0, ISLAND_Y_MAX}, COLOR_GRASS},
};

static void EC_Island_Free(Component* component){
    EC_Island* island = (EC_Island*)component->self;
    Noise_Free(island->noise);
    free(island);
}

static EC_Island* EC_Island_Create(Entity* entity, Noise* noise, EC_MeshRenderer* ec_meshRenderer_island){
    EC_Island* ec_island = malloc(sizeof(EC_Island));
    ec_island->ec_meshRenderer_island = ec_meshRenderer_island;
    ec_island->noise = noise;
    // Component
    ec_island->component = Component_Create(ec_island, entity, EC_T_ISLAND, EC_Island_Free, NULL, NULL, NULL, NULL, NULL);
    return ec_island;
}

EC_Island* Prefab_Island(Entity* parent, TransformSpace TS, V3 position, Quaternion rotation, V3 scale, V3 meshScale){
    LogCreate(&_logConfig, "Prefab_Island");
    Entity* entity = Entity_Create(parent, true, "Island", TS, position, rotation, scale);
    float noiseWidth = 400;
    // Island Renderer
    NoiseModifier modifiers[] = {
        {
            .function = Noise_Modifier_Mask_Circle,
            .argCount = 1,
            .args = (void*[]){&(float){noiseWidth * 0.45}}
        }
    };
    // Noise
    Noise* noise = Noise_Create(noiseWidth, noiseWidth, 40, ISLAND_Y_MIN, ISLAND_Y_MAX, 1, modifiers);
    Noise_RecalculateMap(noise);
    // Create Mesh
    Mesh* islandMesh = Noise_CreateMesh(noise, meshScale, ISLAND_LAYERS_SIZE, ISLAND_LAYERS, true, NULL, 10, (V3){0.5, 0, 0.5});
    // create Collider
    ColliderData colliderData = {.mesh.mesh = islandMesh};
    EC_Collider* ec_collider = EC_Collider_Create(entity, V3_ZERO, false, EC_COLLIDER_MESH, colliderData);
    // Temporary fix: add a rigidbody to the island so we can see it
    // EC_RigidBody* ec_rigidbody = EC_RigidBody_Create(entity, ec_collider, 0.0f, false, RigidBodyConstraints_FreezePosition());
    // Material
    Shader* islandShader = ShaderManager_Get(SHADER_ISLAND);
    Material* islandMaterial = Material_Create(islandShader, 0, NULL);
    EC_MeshRenderer* ec_meshRenderer_island = EC_MeshRenderer_Create(entity, islandMesh, meshScale, islandMaterial);
    // Island
    EC_Island* e_island = EC_Island_Create(entity, noise, ec_meshRenderer_island);
    return e_island;
}

inline V3 EC_Island_GetPositionOnLand(EC_Island* ec_island, V3 position){
    Noise* noise = ec_island->noise;
    EC_MeshRenderer *ec_meshRenderer = ec_island->ec_meshRenderer_island;
    V3 islandPos = EC_WPos(ec_island->component);
    V3 positionIslandSpace = V3_SUB(position, islandPos);
    float yIslandSpace = Noise_GetMeshHeightAt(noise, positionIslandSpace, ec_meshRenderer->meshScale, ec_meshRenderer->mesh->pivot);
    return (V3){position.x, yIslandSpace + islandPos.y, position.z};
}