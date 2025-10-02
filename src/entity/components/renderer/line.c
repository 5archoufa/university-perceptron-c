#include "entity/components/renderer/line.h"
#include "entity/components/renderer/renderer.h"
#include <stdint.h>

static void RD_Line_Free(EC_Renderer *renderer)
{
    RD_Line *line = (RD_Line *)renderer->renderData;
    free(line);
}

static void RD_Line_UpdateBounds(EC_Renderer *renderer)
{
    RD_Line *line = (RD_Line *)renderer->renderData;
    renderer->bounds.dimensions = V2_SIZE(&line->start, &line->end);
    V2 mid = V2_MID(&line->start, &line->end);
    renderer->bounds.position = (V3){mid.x, mid.y, line->z};
}

static inline void SetThickPixel(uint32_t *pixels, int x, int y, int image_w, int image_h, int width, uint32_t color)
{
    int half = width * 0.5;
    for (int dy = -half; dy <= half; dy++)
    {
        int yy = y + dy;
        if (yy < 0 || yy >= image_h)
            continue;

        for (int dx = -half; dx <= half; dx++)
        {
            int xx = x + dx;
            // If either coordinate is out of bounds, skip this pixel and continue
            if (xx < 0 || xx >= image_w || yy < 0 || yy >= image_h)
            {
                break;
            }
            int pixelIndex = yy * image_w + xx;
            pixels[pixelIndex] = Color_Over(color, pixels[pixelIndex]);
        }
    }
}

static void RD_Line_Render(EC_Camera *camera, EC_Renderer *renderer)
{
    // Image data
    uint32_t *pixels = (uint32_t *)camera->image->data;
    int image_w = camera->image->width, image_h = camera->image->height;

    // Line data
    RD_Line *line = (RD_Line *)renderer->renderData;
    V2_INT lineStart = Camera_WorldToScreen_V2(camera, &line->start);
    V2_INT lineEnd = Camera_WorldToScreen_V2(camera, &line->end);

    // Draw line using Bresenham's line algorithm
    int dx = abs(lineEnd.x - lineStart.x);
    int dy = abs(lineEnd.y - lineStart.y);
    int sx = lineStart.x < lineEnd.x ? 1 : -1;
    int sy = lineStart.y < lineEnd.y ? 1 : -1;
    int err = dx - dy;
    while (true)
    {
        SetThickPixel(pixels, lineStart.x, lineStart.y, image_w, image_h, line->width, line->pixelColor);
        if (lineStart.x == lineEnd.x && lineStart.y == lineEnd.y)
            break;
        int err2 = err * 2;
        if (err2 >= -dy)
        {
            err -= dy;
            lineStart.x += sx;
        }
        if (err2 < dx)
        {
            err += dx;
            lineStart.y += sy;
        }
    }
}

EC_Renderer *RD_Line_CreateWithRenderer(Entity *entity, V2 start, V2 end, float z, float width, uint32_t pixelColor)
{
    RD_Line *line = malloc(sizeof(RD_Line));
    line->start = start;
    line->end = end;
    line->z = z;
    line->width = width;
    line->pixelColor = pixelColor;
    EC_Renderer *EC_renderer_line = EC_Renderer_Create(entity, line, RD_Line_Render, RD_Line_Free, RD_Line_UpdateBounds);
    line->EC_renderer_line = EC_renderer_line;
    return EC_renderer_line;
}

EC_Renderer *Prefab_Line(Entity *parent, V2 start, V2 end, float z)
{
    // Entity
    Entity *E_line = Entity_Create(parent, "Line", (V3){0.0, 0.0, z}, 0.0, V2_ONE, V2_HALF);
    // Line Render Data
    RD_Line *RD_line = malloc(sizeof(RD_Line));
    RD_line->start = start;
    RD_line->end = end;
    RD_line->z = z;
    // Renderer
    EC_Renderer *EC_renderer_line = EC_Renderer_Create(E_line, RD_line, RD_Line_Render, RD_Line_Free, RD_Line_UpdateBounds);
    RD_line->EC_renderer_line = EC_renderer_line;
    return EC_renderer_line;
}