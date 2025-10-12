#include "entity/components/renderer/rd_island.h"
#include "entity/components/renderer/renderer.h"
#include "entity/entity.h"
#include <math.h>
#include <stdlib.h>

static void RD_Island_Render(EC_Camera *camera, EC_Renderer *ec_renderer)
{
    // RD_Island *rd_island = (RD_Island *)ec_renderer->renderData;
    // V2_INT startPosition = Camera_WorldToScreen_V3(camera, &ec_renderer->bounds.position);
    // uint32_t *pixels = (uint32_t *)camera->image->data;
    // size_t cameraWidth = camera->image->width, cameraHeight = camera->image->height;
    // for (int x = 0; x < rd_island->noise->width; x++)
    // {
    //     for (int y = 0; y < rd_island->noise->height; y++)
    //     {
    //         // Find Color
    //         float value = rd_island->noise->map[x][y];
    //         uint32_t color = 0x000000;
    //         for (int i = 0; i < ISLAND_LAYERS_SIZE; i++)
    //         {
    //             if (value >= ISLAND_LAYERS[i].yLevel.x && value < ISLAND_LAYERS[i].yLevel.y)
    //             {
    //                 color = ISLAND_LAYERS[i].color;
    //                 break;
    //             }
    //         }
    //         // Draw pixel
    //         V2_INT horizontal = {
    //             startPosition.x + (x * rd_island->unitsPerCell),
    //             startPosition.x + ((x + 1) * rd_island->unitsPerCell)};
    //         V2_INT vertical = {
    //             startPosition.y + (y * rd_island->unitsPerCell),
    //             startPosition.y + ((y + 1) * rd_island->unitsPerCell)};
    //         for (int i = horizontal.x; i < horizontal.y; i++)
    //         {
    //             for (int j = vertical.x; j < vertical.y; j++)
    //             {
    //                 if (i < 0 || j < 0 || i >= cameraWidth || j >= cameraHeight)
    //                     continue;
    //                 pixels[j * cameraWidth + i] = color;
    //             }
    //         }
    //     }
    // }
}

static void RD_Island_Free(EC_Renderer *renderer)
{
    RD_Island *island = (RD_Island *)renderer->renderData;
    Noise_Free(island->noise);
    free(island);
}

static void RD_Island_UpdateBounds(EC_Renderer *renderer)
{
    RD_Island *island = (RD_Island *)renderer->renderData;
    renderer->bounds.size = (V3){island->noise->width * island->unitsPerCell, 0, island->noise->height * island->unitsPerCell};
    // renderer->bounds.start = renderer->component->entity->transform.position;
    renderer->bounds.max = (V3){
        renderer->bounds.min.x + renderer->bounds.size.x,
        renderer->bounds.min.y,
        renderer->bounds.min.z + renderer->bounds.size.z};
}

// EC_Renderer *RD_Island_CreateWithRenderer(Entity *entity, int width, int height, float interpolation, float unitsPerCell)
// {
//     RD_Island *rd_island = malloc(sizeof(RD_Island));
//     rd_island->ec_renderer = EC_Renderer_Create(entity, rd_island, RD_Island_Render, NULL, RD_Island_Free, RD_Island_UpdateBounds);
//     rd_island->unitsPerCell = unitsPerCell;
//     Noise *noise = Noise_Create(width, height, interpolation, ISLAND_Y_MIN, ISLAND_Y_MAX);
//     rd_island->noise = noise;
//     // Fade the noise out around the island borders
//     for (int x = 0; x < noise->interpWidth; x++)
//     {
//         noise->interpGrid[x][0] = V2_ZERO;
//     }

//     Noise_ForceUpdate(rd_island->noise);
//     return rd_island->ec_renderer;
// }
