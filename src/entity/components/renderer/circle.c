#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "entity/components/renderer/circle.h"
#include "entity/components/renderer/renderer.h"
#include "entity/components/camera/camera.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdint.h>

static void RD_Circle_Printf(RD_Circle *circle)
{
    printf("Circle(name<%s>): {\n  Radius<%f>\n  Dimensions<%f,%f>\n}\n}",
           circle->renderer->component->entity->name,
           circle->radius,
           circle->renderer->bounds.dimensions.x,
           circle->renderer->bounds.dimensions.y);
}

static void RD_Circle_Render(EC_Camera *camera, EC_Renderer *EC_renderer)
{
    //char *pixels = (char *)camera->image->data;
    uint32_t *pixels = (uint32_t*)camera->image->data;
    RD_Circle *circle = EC_renderer->renderData;
    V3 center = *EC_renderer->bounds.position;
    center = V3_SUB(&camera->component->entity->position, &center);
    V2 halfViewport = V2_MUL(&camera->viewport, 0.5);
    center = V3_ADD_V2(&center, &halfViewport);
    int width = camera->image->width, height = camera->image->height;
    for (float a = 0.0; a <= 90.0; a += 0.2)
    {
        float x = sin(a * M_PI / 180.0) * circle->radius;
        float y = cos(a * M_PI / 180.0) * circle->radius;
        V3 ur = {center.x + x, center.y + y, center.z};
        V3 ul = {center.x - x, ur.y, center.z};
        V3 br = {ur.x, center.y - y, center.z};
        V3 bl = {ul.x, br.y, center.z};
        if (ur.x >= 0 && ur.x < width && ur.y > 0 && ur.y < height)
        {
            // char *row = (char *)(camera->image->data + (int)ur.y * camera->image->bytes_per_line);
            // row[(int)ur.x] = PIXEL_WHITE;
            pixels[(int)ur.y * width + (int)ur.x] = PIXEL_WHITE;
            // XPutPixel(camera->image, (int)ur.x, (int)ur.y, PIXEL_BLACK);
        }
        if (ul.x >= 0 && ul.x < width && ul.y > 0 && ul.y < height)
        {
            //XPutPixel(camera->image, ul.x, ul.y, PIXEL_BLACK);
            pixels[(int)ul.y * width + (int)ul.x] = PIXEL_WHITE;
        }
        if (bl.x >= 0 && bl.x < width && bl.y > 0 && bl.y < height)
        {
            //XPutPixel(camera->image, bl.x, bl.y, PIXEL_BLACK);
            pixels[(int)bl.y * width + (int)bl.x] = PIXEL_WHITE;
        }
        if (br.x >= 0 && br.x < width && br.y > 0 && br.y < height)
        {
            //XPutPixel(camera->image, br.x, br.y, PIXEL_BLACK);
            pixels[(int)br.y * width + (int)br.x] = PIXEL_WHITE;
        }
    }
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
    EC_Renderer *renderer = EC_Renderer_Create(entity, circle, RD_Circle_Render, RD_Circle_Free);
    circle->renderer = renderer;
    renderer->bounds.dimensions = (V2){radius * 2, radius * 2};
    return renderer;
}