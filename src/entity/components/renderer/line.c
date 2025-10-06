#include "entity/components/renderer/line.h"
#include "entity/components/renderer/renderer.h"
#include <stdint.h>
#include "utilities/math/stupid_math.h"

static void RD_Line_Free(EC_Renderer *renderer)
{
    RD_Line *line = (RD_Line *)renderer->renderData;
    free(line);
}

static void RD_Line_UpdateBounds(EC_Renderer *renderer)
{
    RD_Line *line = (RD_Line *)renderer->renderData;
    renderer->bounds.start = line->start;
    renderer->bounds.end = line->end;
    renderer->bounds.size = V3_SIZE(line->start, line->end);
}

static inline void SetThickPixel(uint32_t *pixels, int x, int y, Rect* boundary, int size, uint32_t color)
{
    int start = -size * 0.5;
    int end = start + size;
    for (int dy = start; dy <= end; dy++)
    {
        int yy = y + dy;
        if (yy < 0 || yy >= boundary->end.y)
            continue;

        for (int dx = start; dx <= end; dx++)
        {
            int xx = x + dx;
            // If either coordinate is out of bounds, skip this pixel and continue
            if (!IsBetween(xx, boundary->start.x, boundary->end.x) || !IsBetween(yy, boundary->start.y, boundary->end.y))
            {
                break;
            }
            int pixelIndex = yy * boundary->end.x + xx;
            pixels[pixelIndex] = Color_Over(color, pixels[pixelIndex]);
        }
    }
}

static void RD_Line_Render3D(EC_Camera *camera, EC_Renderer *renderer, Rect unboundedRectangle, Rect boundedRectangle)
{
    // Image data
    uint32_t *pixels = (uint32_t *)camera->image->data;

    // Line data
    RD_Line *line = (RD_Line *)renderer->renderData;
    int startX = unboundedRectangle.start.x;
    int startY = unboundedRectangle.start.y;
    int endX = unboundedRectangle.end.x;
    int endY = unboundedRectangle.end.y;

    // Draw line using Bresenham's line algorithm
    int dx = abs(endX - startX);
    int dy = abs(endY - startY);
    int sx = startX < endX ? 1 : -1;
    int sy = startY < endY ? 1 : -1;
    int err = dx - dy;
    while (true)
    {
        if (IsBetween(startX, boundedRectangle.start.x, boundedRectangle.end.x) && IsBetween(startY, boundedRectangle.start.y, boundedRectangle.end.y))
        {
            //SetThickPixel(pixels, startX, startY, &boundedRectangle, 1, line->pixelColor);
            pixels[startY * camera->image->width + startX] = Color_Over(line->color, pixels[startY * camera->image->width + startX]);
        }
        if (startX == endX && startY == endY)
            break;
        int err2 = err * 2;
        if (err2 >= -dy)
        {
            err -= dy;
            startX += sx;
        }
        if (err2 < dx)
        {
            err += dx;
            startY += sy;
        }
    }
}

static void RD_Line_Render(EC_Camera *camera, EC_Renderer *renderer)
{
    // // Image data
    // uint32_t *pixels = (uint32_t *)camera->image->data;
    // int image_w = camera->image->width, image_h = camera->image->height;

    // // Line data
    // RD_Line *line = (RD_Line *)renderer->renderData;
    // V2_INT lineStart = Camera_WorldToScreen_V2(camera, &line->start);
    // V2_INT lineEnd = Camera_WorldToScreen_V2(camera, &line->end);

    // // Draw line using Bresenham's line algorithm
    // int dx = abs(lineEnd.x - lineStart.x);
    // int dy = abs(lineEnd.y - lineStart.y);
    // int sx = lineStart.x < lineEnd.x ? 1 : -1;
    // int sy = lineStart.y < lineEnd.y ? 1 : -1;
    // int err = dx - dy;
    // while (true)
    // {
    //     SetThickPixel(pixels, lineStart.x, lineStart.y, image_w, image_h, line->thickness, line->pixelColor);
    //     if (lineStart.x == lineEnd.x && lineStart.y == lineEnd.y)
    //         break;
    //     int err2 = err * 2;
    //     if (err2 >= -dy)
    //     {
    //         err -= dy;
    //         lineStart.x += sx;
    //     }
    //     if (err2 < dx)
    //     {
    //         err += dx;
    //         lineStart.y += sy;
    //     }
    // }
}

EC_Renderer *RD_Line_CreateWithRenderer(Entity *entity, V3 start, V3 end, float thickness, uint32_t pixelColor)
{
    RD_Line *line = malloc(sizeof(RD_Line));
    line->start = start;
    line->end = end;
    line->thickness = thickness;
    line->color = pixelColor;
    // EC_Renderer *EC_renderer_line = EC_Renderer_Create(entity, line, RD_Line_Render, RD_Line_Render3D, RD_Line_Free, RD_Line_UpdateBounds);
    // line->EC_renderer_line = EC_renderer_line;
    return NULL;
}

EC_Renderer *Prefab_Line(Entity *parent, V3 start, V3 end, float thickness, uint32_t color)
{
    // Entity
    Entity *E_line = Entity_Create(parent, "Line", TS_WORLD, V3_ZERO, QUATERNION_IDENTITY, V3_ONE);
    // Line Render Data
    RD_Line *RD_line = malloc(sizeof(RD_Line));
    RD_line->start = start;
    RD_line->end = end;
    RD_line->thickness = thickness;
    RD_line->color = color;
    // Renderer
    // EC_Renderer *EC_renderer_line = EC_Renderer_Create(E_line, RD_line, RD_Line_Render, NULL, RD_Line_Free, RD_Line_UpdateBounds);
    // RD_line->EC_renderer_line = EC_renderer_line;
    return NULL;
}