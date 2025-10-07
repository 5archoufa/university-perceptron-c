#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "entity/components/renderer/circle.h"
#include "entity/components/renderer/renderer.h"
#include "entity/components/camera/camera.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdint.h>
#include "utilities/math/stupid_math.h"

static void RD_Circle_UpdateBounds(EC_Renderer *renderer)
{
    RD_Circle *circle = (RD_Circle *)renderer->renderData;
    renderer->bounds.size = (V3){circle->radius * 2, circle->radius * 2, 0};
    V3 position = EC_WPos(renderer->component);
    renderer->bounds.start = (V3){
        position.x,
        position.y,
        position.z};
    renderer->bounds.end = (V3){
        position.x + circle->radius * 2,
        position.y + circle->radius * 2,
        position.z};
}

static void RD_Circle_Printf(RD_Circle *circle)
{
    printf("Circle(name<%s>): {\n  Radius<%f>\n  Dimensions<%f,%f>\n}\n}",
           circle->EC_renderer_circle->component->entity->name,
           circle->radius,
           circle->EC_renderer_circle->bounds.size.x,
           circle->EC_renderer_circle->bounds.size.y);
}

static void RD_Circle_Render(EC_Camera *camera, EC_Renderer *EC_renderer)
{
    // // char *pixels = (char *)camera->image->data;
    // uint32_t *pixels = (uint32_t *)camera->image->data;
    // RD_Circle *circle = EC_renderer->renderData;
    // V2_INT center = Camera_WorldToScreen_V3(camera, &EC_renderer->bounds.position);
    // int width = camera->image->width, height = camera->image->height;
    // for (int y = center.y - circle->radius; y < center.y + circle->radius; y++)
    // {
    //     float dy = (y - center.y);
    //     float dx = sqrt((circle->radius * circle->radius) - (dy * dy));
    //     for (int x = center.x - dx; x < center.x + dx; x++)
    //     {
    //         if (x >= 0 && x < width && y > 0 && y < height)
    //         {
    //             pixels[y * width + x] = PIXEL_WHITE;
    //         }
    //     }
    // }
}

static void RD_Circle_Render3D(EC_Camera *camera, EC_Renderer *EC_renderer, Rect unboundedRectangle, Rect boundedRectangle)
{
}

static void RD_Circle_Free(EC_Renderer *renderer)
{
    RD_Circle *circle = renderer->renderData;
    free(circle);
}

EC_Renderer *RD_Circle_CreateWithRenderer(Entity *entity, float radius)
{
    RD_Circle *circle = malloc(sizeof(RD_Circle));
    circle->radius = radius;
    // EC_Renderer *EC_renderer_circle = EC_Renderer_Create(entity, circle, RD_Circle_Render, RD_Circle_Render3D, RD_Circle_Free, RD_Circle_UpdateBounds);
    // circle->EC_renderer_circle = EC_renderer_circle;
    return NULL;
}