#include "entity/components/renderer/rd_noise.h"
#include "entity/components/renderer/renderer.h"
#include "entity/components/camera/camera.h"
#include "entity/entity.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>


static void RD_Noise_Render(EC_Camera *camera, EC_Renderer *renderer)
{
    RD_Noise *rd_noise = renderer->renderData;
    Noise *noise = rd_noise->noise;
    uint32_t *pixels = (uint32_t *)camera->image->data;
    V2_INT noiseScreenPos = Camera_WorldToScreen_V3(camera, &renderer->bounds.position);
    for (int i = 0; i < noise->width; i++)
    {
        for (int j = 0; j < noise->height; j++)
        {
            int x = noiseScreenPos.x + i * camera->pixelsPerMeter;
            int y = noiseScreenPos.y + j * camera->pixelsPerMeter;
            if (x >= 0 && x < camera->image->width && y >= 0 && y < camera->image->height)
            {
                float value = noise->map[i][j];
                // printf("%f\n", 1.0/value);
                pixels[y * camera->image->width + x] = Color_Modulate_Grayscale(PIXEL_WHITE, value);
            }
        }
    }
}

static void RD_Noise_Free(EC_Renderer *renderer)
{
    RD_Noise *rd_noise = renderer->renderData;
    free(rd_noise);
}

static void RD_Noise_UpdateBounds(EC_Renderer *ec_renderer)
{
    RD_Noise *rd_noise = ec_renderer->renderData;
    V2 noiseSize = (V2){rd_noise->noise->width, rd_noise->noise->height};
    ec_renderer->bounds.dimensions = V2_MUL(&noiseSize, &ec_renderer->component->entity->scale);
    V3 position = ec_renderer->component->entity->position;
    ec_renderer->bounds.position = position;
}

EC_Renderer *Prefab_Noise(Entity *parent, V3 position, float rotation, V2 scale, V2 pivot, Noise *noise)
{
    Entity *e_noise = Entity_Create(parent, "Noise", position, 0.0, scale, pivot);
    EC_Renderer *ec_renderer = RD_Noise_CreateWithRenderer(e_noise, noise);
    return ec_renderer;
}

EC_Renderer *RD_Noise_CreateWithRenderer(Entity *entity, Noise *noise)
{
    // Create RD_Noise
    RD_Noise *rd_noise = malloc(sizeof(RD_Noise));
    rd_noise->noise = noise;
    // Create Renderer
    EC_Renderer *renderer = EC_Renderer_Create(entity, rd_noise, RD_Noise_Render, RD_Noise_Free, RD_Noise_UpdateBounds);
    rd_noise->renderer = renderer;
    return renderer;
}