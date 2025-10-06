#include "entity/entity.h"
#include "entity/components/ec_water/ec_water.h"

#define COLOR_WATER_DARKEST 0xff8a382a // #2a388aff
#define COLOR_WATER_DARK 0xffba513f    // #3f51baff
#define COLOR_WATER_LIGHT 0xffff7961   // #6179ffff

static const float WATER_NOISE_MIN = -10.0f;
static const float WATER_NOISE_MAX = 0.0f;

static NoiseLayer WATER_NOISE_LAYERS[] = {
    {{-3, WATER_NOISE_MAX}, COLOR_WATER_LIGHT},
    {{-5.0, -3}, COLOR_WATER_DARK},
    {{WATER_NOISE_MIN, -5}, COLOR_WATER_DARKEST},
};

static void EC_Water_Free(Component *component)
{
    EC_Water *ec_water = component->self;
    Noise_Free(ec_water->noise);
    free(ec_water);
}

static EC_Water *EC_Water_Create(Entity *entity, EC_Renderer3D *ec_renderer3d_water, Noise *noise)
{
    EC_Water *ec_water = malloc(sizeof(EC_Water));
    ec_water->waveSpeed = 1.0f;
    ec_water->waveHeight = 0.5f;
    ec_water->waveFrequency = 0.2f;
    ec_water->ec_renderer3d_water = ec_renderer3d_water;
    ec_water->noise = noise;
    // Component
    ec_water->component = Component_Create(ec_water, entity, EC_T_WATER, EC_Water_Free, NULL, NULL, NULL, NULL, NULL);
    return ec_water;
}

EC_Water *Prefab_Water(Entity *e_parent, TransformSpace TS, V3 position, Quaternion rotation, V3 scale, V3 meshScale, size_t modifiers_size, NoiseModifier* modifiers)
{
    Entity *e_water = Entity_Create(e_parent, "Water", TS, position, rotation, scale);
    Noise *noise = Noise_Create(200, 200, 3, WATER_NOISE_MIN, WATER_NOISE_MAX, modifiers_size, modifiers);
    Noise_RecalculateMap(noise);
    Texture* texture = Texture_CreateEmpty();
    Mesh *mesh = Noise_CreateMesh(noise, meshScale, 3, WATER_NOISE_LAYERS, true, NULL, 10);
    Texture_UploadToGPU(texture);
    // Renderer3D
    EC_Renderer3D *ec_renderer3d_water = EC_Renderer3D_Create(e_water, mesh, NULL);
    EC_Water *ec_water = EC_Water_Create(e_water, ec_renderer3d_water, noise);
    return ec_water;
}