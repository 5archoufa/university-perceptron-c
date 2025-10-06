#include "entity/components/island/island.h"
#include "entity/components/renderer/rd_island.h"
#include "input/input_manager.h"
#include "entity/components/ec_renderer3d/ec_renderer3d.h"
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include "logging/logger.h"
#include "entity/transform.h"

static LogConfig _logConfig = {"EC_Island", LOG_LEVEL_INFO, LOG_COLOR_BLUE};

static const int INPUT_INTER_UP = 0;
static const int INPUT_INTER_DOWN = 1;

// Colors must be in ABGR format
static const uint32_t COLOR_GRASS = 0xff32963f;    // #3f9632
static const uint32_t COLOR_STONE = 0xff22262a;    // #2a2622ff
static const uint32_t COLOR_SAND = 0xff7cdaeb;     // #ebda7cff
static const uint32_t COLOR_MOUNTAIN = 0xff20293c; // #3c2920ff

static const size_t ISLAND_LAYERS_SIZE = 4;
static const float ISLAND_Y_MIN = -19;
static const float ISLAND_Y_MAX = 20;
static const NoiseLayer ISLAND_LAYERS[] = {
    {{ISLAND_Y_MIN, -3}, COLOR_STONE},
    {{-3, 0.0}, COLOR_SAND},
    {{0.0, 6.0}, COLOR_GRASS},
    {{6.0, ISLAND_Y_MAX}, COLOR_MOUNTAIN},
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
    float noiseWidth = 500;
    // Island Renderer
    NoiseModifier modifiers[] = {
        {
            .function = Noise_Modifier_Mask_Circle,
            .argCount = 1,
            .args = (void*[]){&(float){noiseWidth * 0.45}}
        }
    };
    Noise* noise = Noise_Create(noiseWidth, noiseWidth, 10, ISLAND_Y_MIN, ISLAND_Y_MAX, 1, modifiers);
    Noise_RecalculateMap(noise);
    Log(&_logConfig, "Recalculating Noise Map: %dx%d", noise->width, noise->height);
    // Drop the interpolation in the borders to create an island effect
    Texture* texture = Texture_CreateEmpty();
    Mesh* islandMesh = Noise_CreateMesh(noise, meshScale, ISLAND_LAYERS_SIZE, ISLAND_LAYERS, true, NULL, 10);
    Texture_UploadToGPU(texture);
    EC_Renderer3D* ec_renderer3d_island = EC_Renderer3D_Create(entity, islandMesh, NULL);
    // Island
    EC_Island* e_island = EC_Island_Create(entity, ec_renderer3d_island);
    return e_island;
}