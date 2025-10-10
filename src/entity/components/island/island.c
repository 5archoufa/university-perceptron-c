#include "entity/components/island/island.h"
#include "entity/components/renderer/rd_island.h"
#include "input/input_manager.h"
#include "entity/components/ec_renderer3d/ec_renderer3d.h"
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
    InputContext_Free(island->inputContext);
    free(island);
}

static void EC_Island_Update(Component* component){
    EC_Island* island = (EC_Island*)component->self;
    if(island->inputContext->keys[INPUT_INTER_UP].isPressed){
        
    }else if(island->inputContext->keys[INPUT_INTER_DOWN].isPressed){
        
    }
}

static EC_Island* EC_Island_Create(Entity* entity, EC_Renderer3D* ec_renderer3d_island){
    EC_Island* ec_island = malloc(sizeof(EC_Island));
    ec_island->ec_renderer3d_island = ec_renderer3d_island;
    // Input Context
    int keys[] = {
        GLFW_KEY_8,
        GLFW_KEY_2
    };
    InputContext* inputContext = InputContext_Create("IslandInputContext",
                                                         2,
                                                         keys,
                                                         0,
                                                         NULL,
                                                         0,
                                                         NULL);
    ec_island->inputContext = inputContext;
    // Component
    ec_island->component = Component_Create(ec_island, entity, EC_T_ISLAND, EC_Island_Free, NULL, NULL, EC_Island_Update, NULL, NULL);
    return ec_island;
}

EC_Island* Prefab_Island(Entity* parent, TransformSpace TS, V3 position, Quaternion rotation, V3 scale, V3 meshScale){
    LogCreate(&_logConfig, "Prefab_Island");
    Entity* entity = Entity_Create(parent, "Island", TS, position, rotation, scale);
    float noiseWidth = 1000;
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
    // Material
    Shader* islandShader = ShaderManager_Get(SHADER_TOON_SOLID);
    Material* islandMaterial = Material_Create(islandShader, 0, NULL);
    // Renderer
    EC_Renderer3D* ec_renderer3d_island = EC_Renderer3D_Create(entity, islandMesh, islandMaterial);
    // Island
    EC_Island* e_island = EC_Island_Create(entity, ec_renderer3d_island);
    // Print out the island's components
    for(int i = 0; i < entity->componentCount; i++){
        Component* comp = entity->component_values[i];
        Log(&_logConfig, "Island's component %d: Type %d, Free: %p", i, comp->type, comp->Free);
    }
    return e_island;
}