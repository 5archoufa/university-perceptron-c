#include "utilities/noise/noise.h"
#include "utilities/math/stupid_math.h"
#include "utilities/math/v2.h"
#include <stdarg.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include "rendering/texture/texture.h"
#include "entity/components/renderer/renderer.h"

typedef enum
{
    BOTTOM_LEFT,
    BOTTOM_RIGHT,
    TOP_LEFT,
    TOP_RIGHT
} Corner;

static NoiseLayer NOISE_LAYER_DEFAULT = {
    .yLevel = {0.0f, 0.0f},
    .color = 0xFFFFFFFF // White #ffffffffFF
};

static inline V2_INT InterpCordsToPixel(Noise *noise, V2_INT interpCords)
{
    V2_INT pixel = (V2_INT){interpCords.x * noise->interp, interpCords.y * noise->interp};
    return pixel;
}

static inline V2 GetInterpValue(Noise *noise, int pixelX, int pixelY, Corner corner, V2_INT *out_interpPixelCords)
{
    V2_INT interpCords = {pixelX / noise->interp, pixelY / noise->interp};
    switch (corner)
    {
    case BOTTOM_RIGHT:
        interpCords.x++;
        break;
    case TOP_LEFT:
        interpCords.y++;
        break;
    case TOP_RIGHT:
        interpCords.x++;
        interpCords.y++;
        break;
    }

    *out_interpPixelCords = InterpCordsToPixel(noise, interpCords);
    return noise->interpGrid[interpCords.x][interpCords.y];
}

static float Fade(float t)
{
    return t * t * (3 - 2 * t);
}

void Noise_Modifier_Mask_Circle(Noise *noise, int argCount, void **args)
{
    if (argCount < 1)
    {
        printf("Noise_Modifier_Mask_Circle requires at least 1 argument (Radius).\n");
        return;
    }
    float radius = *(float *)args[0];
    printf("Applying Circle Mask with radius %.2f\n", radius);
    int centerX = noise->width / 2;
    int centerY = noise->height / 2;
    float maxDistance = fminf(centerX, centerY);
    float dropStart = radius;    // start dropping at this radius
    float dropEnd = maxDistance; // fully at min at this distance

    for (int x = 0; x < noise->width; x++)
    {
        for (int y = 0; y < noise->height; y++)
        {
            float dx = x - centerX;
            float dy = y - centerY;
            float distance = sqrtf(dx * dx + dy * dy);

            if (distance > dropStart)
            {
                float t = (distance - dropStart) / (dropEnd - dropStart);
                t = fminf(fmaxf(t, 0.0f), 1.0f); // clamp 0..1
                noise->map[x][y] = noise->map[x][y] * (1 - t) + noise->min * t;
            }
        }
    }
}

void Noise_AddModifier(Noise *noise, NoiseModifier modifier)
{
    noise->modifiers_size++;
    noise->modifiers = realloc(noise->modifiers, sizeof(NoiseModifier) * noise->modifiers_size);
    noise->modifiers[noise->modifiers_size - 1] = modifier;
}

/// @brief This simply allocates the Noise and configures it. You must call Noise_Generate(Noise*) to get visual results.
Noise *Noise_Create(int width, int height, float interpolation, float min, float max, size_t modifiers_size, NoiseModifier *modifiers)
{
    // Pull out modifiers from variadic arguments

    // Allocate Noise
    Noise *noise = malloc(sizeof(Noise));
    noise->width = width;
    noise->height = height;
    noise->interp = interpolation;
    noise->min = min;
    noise->max = max;
    noise->modifiers_size = modifiers_size;
    if (modifiers_size == 0)
    {
        noise->modifiers = NULL;
    }
    else
    {
        noise->modifiers = malloc(sizeof(NoiseModifier) * modifiers_size);
        for (size_t i = 0; i < modifiers_size; i++)
        {
            noise->modifiers[i] = modifiers[i];
        }
    }
    // Allocate Map
    noise->map = malloc(width * sizeof(float *));
    for (int x = 0; x < width; x++)
    {
        noise->map[x] = malloc(height * sizeof(float));
        for (int y = 0; y < height; y++)
        {
            noise->map[x][y] = 0.0;
        }
    }
    // Allocate Interpolation Grid
    int interpWidth = ceil(width / interpolation) + 1;
    noise->interpWidth = interpWidth;
    int interpHeight = ceil(height / interpolation) + 1;
    noise->interpHeight = interpHeight;
    noise->interpGrid = malloc(sizeof(V2 *) * interpWidth);
    // Fill Interpolation Grid with vectors
    for (int x = 0; x < interpWidth; x++)
    {
        noise->interpGrid[x] = malloc(sizeof(V2) * interpHeight);
        for (int y = 0; y < interpHeight; y++)
        {
            noise->interpGrid[x][y] = V2_Rand(1.0);
        }
    }
    Noise_Update(noise, width, height, interpolation, min, max, false);
    return noise;
}

void Noise_Regenerate(Noise *noise)
{
    Noise_Update(noise, noise->width, noise->height, noise->interp, noise->min, noise->max, true);
}

void Noise_RecalculateMap(Noise *noise)
{
    float theoreticalMin = -sqrtf(2.0f) * noise->interp;
    float theoreticalMax = sqrtf(2.0f) * noise->interp;
    for (int x = 0; x < noise->width; x++)
    {
        for (int y = 0; y < noise->height; y++)
        {
            V2 pixel = {x, y};
            // Compute Dot Products
            V2_INT tl;
            V2_INT tr;
            V2 tl_vector = GetInterpValue(noise, x, y, TOP_LEFT, &tl);
            V2 tr_vector = GetInterpValue(noise, x, y, TOP_RIGHT, &tr);
            V2 tl_pixel = (V2){pixel.x - tl.x, pixel.y - tl.y};
            V2 tr_pixel = (V2){pixel.x - tr.x, pixel.y - tr.y};
            float tl_dot = V2_DOT(&tl_vector, &tl_pixel);
            float tr_dot = V2_DOT(&tr_vector, &tr_pixel);
            V2_INT bl;
            V2_INT br;
            V2 bl_vector = GetInterpValue(noise, x, y, BOTTOM_LEFT, &bl);
            V2 br_vector = GetInterpValue(noise, x, y, BOTTOM_RIGHT, &br);
            V2 bl_pixel = (V2){pixel.x - bl.x, pixel.y - bl.y};
            V2 br_pixel = (V2){pixel.x - br.x, pixel.y - br.y};
            float bl_dot = V2_DOT(&bl_vector, &bl_pixel);
            float br_dot = V2_DOT(&br_vector, &br_pixel);
            // SX SY
            float sx = (float)(x % noise->interp) / (float)noise->interp;
            float sy = (float)(y % noise->interp) / (float)noise->interp;
            // Fade
            float u = Fade(sx);
            float v = Fade(sy);
            // Interpolate
            float bottom = bl_dot * (1 - u) + br_dot * u;
            float top = tl_dot * (1 - u) + tr_dot * u;
            float value = bottom * (1 - v) + top * v;

            value = (value - theoreticalMin) / (theoreticalMax - theoreticalMin);
            value = noise->min + value * (noise->max - noise->min);
            noise->map[x][y] = value;
        }
    }
    // Apply Modifiers
    for (int i = 0; i < noise->modifiers_size; i++)
    {
        noise->modifiers[i].function(noise, noise->modifiers[i].argCount, noise->modifiers[i].args);
    }
}

void Noise_Update(Noise *noise, int width, int height, float interpolation, float min, float max, bool forceUpdate)
{
    bool updateInterpGrid = forceUpdate || false;
    bool updateMap = forceUpdate || false;
    if (noise->width != width || noise->height != height)
    {
        updateMap = true;
        updateInterpGrid = true;
    }
    if (interpolation != noise->interp)
    {
        updateInterpGrid = true;
        updateMap = true;
    }
    if (min != noise->min || max != noise->max)
    {
        updateMap = true;
    }

    if (updateMap)
    {
        noise->max = max;
        noise->min = min;
        noise->width = width;
        noise->height = height;
        noise->map = realloc(noise->map, width * sizeof(float *));
        for (int x = 0; x < width; x++)
        {
            noise->map[x] = realloc(noise->map[x], height * sizeof(float));
        }
        noise->width = width;
        noise->height = height;
    }
    if (updateInterpGrid)
    {
        noise->interp = interpolation;
        int interpWidth = ceil(width / interpolation) + 1;
        int interpHeight = ceil(height / interpolation) + 1;
        noise->interpGrid = realloc(noise->interpGrid, sizeof(V2 *) * interpWidth);
        for (int x = 0; x < interpWidth; x++)
        {
            noise->interpGrid[x] = realloc(noise->interpGrid[x], sizeof(V2) * interpHeight);
            for (int y = 0; y < interpHeight; y++)
            {
                noise->interpGrid[x][y] = V2_Rand(1.0);
            }
        }
        noise->interpWidth = interpWidth;
        noise->interpHeight = interpHeight;
    }

    if (updateInterpGrid || updateMap)
    {
        Noise_RecalculateMap(noise);
    }
}

void Noise_Free(Noise *noise)
{
    if (!noise)
    {
        return;
    }
    for (int x = 0; x < noise->width; x++)
    {
        free(noise->map[x]);
    }
    free(noise->map);
    for (int x = 0; x < noise->interpWidth; x++)
    {
        free(noise->interpGrid[x]);
    }
    free(noise->interpGrid);
    free(noise->modifiers);
    free(noise);
}

static NoiseLayer *NoiseLayer_Get(Noise *noise, size_t layers_size, const NoiseLayer *layers, int x, int y)
{
    int lowestLevel = layers[0].yLevel.x;
    int lowestLayer = 0;
    float yLevel = noise->map[x][y];
    for (int i = 0; i < layers_size; i++)
    {
        if (yLevel >= layers[i].yLevel.x && yLevel < layers[i].yLevel.y)
        {
            return &layers[i];
        }
        if (layers[i].yLevel.x < lowestLevel)
        {
            lowestLevel = layers[i].yLevel.x;
            lowestLayer = i;
        }
    }
    printf("Lowest level: %d\n", lowestLevel);
    return &layers[lowestLayer];
}

Mesh *Noise_CreateMesh(Noise *noise, V3 meshScale, int layers_size, const NoiseLayer *layers, bool usePixelColors, Texture *texture, int density, V3 pivot)
{
    float dx = meshScale.x / (float)noise->width;
    float dy = meshScale.y / (float)(noise->max - noise->min);
    float dz = meshScale.z / (float)noise->height;
    float xOffset = -pivot.x * meshScale.x;
    float yOffset = -pivot.y * meshScale.y;
    float zOffset = -pivot.z * meshScale.z;
    int noise_width = noise->width, noise_height = noise->height;
    // Vertices
    uint32_t vertices_size = noise->width * noise->height;
    Vertex *vertices = malloc(sizeof(Vertex) * vertices_size);
    int texture_width = texture ? density * noise_width : noise_width;
    int texture_height = texture ? density * noise_height : noise_height;
    if (texture != NULL)
    {
        printf("Resizing texture for noise mesh: %dx%d\n", noise_width, noise_height);
        Texture_Resize(texture, texture_width, texture_height);
    }
    for (int y = 0; y < noise_height; y++)
    {
        for (int x = 0; x < noise_width; x++)
        {
            vertices[y * noise_width + x].position = (V3){
                x * dx + xOffset,
                noise->map[x][y] * dy + yOffset,
                y * dz + zOffset};

            // Compute normals using central differences (much more accurate)
            float hL = (x > 0) ? noise->map[x - 1][y] : noise->map[x][y];
            float hR = (x < noise_width - 1) ? noise->map[x + 1][y] : noise->map[x][y];
            float hD = (y > 0) ? noise->map[x][y - 1] : noise->map[x][y];
            float hU = (y < noise_height - 1) ? noise->map[x][y + 1] : noise->map[x][y];
            
            // Compute tangent vectors
            V3 tangentX = {2.0f * meshScale.x, (hR - hL) * meshScale.y, 0.0f};
            V3 tangentZ = {0.0f, (hU - hD) * meshScale.y, 2.0f * meshScale.z};
            
            // Normal is cross product of tangents: tangentZ × tangentX (flipped for upward normal)
            V3 normal = V3_CROSS(tangentZ, tangentX);
            vertices[y * noise_width + x].normal = V3_NORM(normal);
            
            // Debug: Print first normal to verify direction
            if (x == noise_width/2 && y == noise_height/2) {
                printf("Center normal: (%.3f, %.3f, %.3f)\n", 
                    vertices[y * noise_width + x].normal.x,
                    vertices[y * noise_width + x].normal.y,
                    vertices[y * noise_width + x].normal.z);
            }
            
            // UVs
            vertices[y * noise_width + x].uv = (UV){
                (float)x / (float)(noise_width - 1),
                (float)y / (float)(noise_height - 1)};
            
            if (usePixelColors)
            {
                uint32_t color = NoiseLayer_Get(noise, layers_size, layers, x, y)->color;
                vertices[y * noise_width + x].color = color;
            }
            else
            {
                vertices[y * noise_width + x].color = PIXEL_WHITE;
            }
        }
    }
    
    if (texture)
    {
        V2 textureNoiseRatio = {(float)noise_width / (float)texture_width, (float)noise_height / (float)texture_height};
        for (int ty = 0; ty < texture_height; ty++)
        {
            for (int tx = 0; tx < texture_width; tx++)
            {
                // Fractional noise coordinates
                float noise_x = (float)tx * textureNoiseRatio.x;
                float noise_y = (float)ty * textureNoiseRatio.y;

                int x0 = (int)noise_x;
                int y0 = (int)noise_y;
                int x1 = (x0 + 1 < noise_width) ? x0 + 1 : x0;
                int y1 = (y0 + 1 < noise_height) ? y0 + 1 : y0;

                float fx = noise_x - x0;
                float fy = noise_y - y0;

                float h = noise->map[x0][y0] * (1 - fx) * (1 - fy) +
                          noise->map[x1][y0] * fx * (1 - fy) +
                          noise->map[x0][y1] * (1 - fx) * fy +
                          noise->map[x1][y1] * fx * fy;

                // Find layer for interpolated height
                uint32_t color = NOISE_LAYER_DEFAULT.color;
                for (int i = 0; i < layers_size; i++)
                {
                    if (h >= layers[i].yLevel.x && h < layers[i].yLevel.y)
                    {
                        color = layers[i].color;
                        break;
                    }
                }
                texture->data[ty * texture_width + tx] = color;
            }
        }
        Texture_UploadToGPU(texture);
    }

    // Indices
    uint32_t indices_size = (noise_width - 1) * (noise_height - 1) * 6;
    uint32_t *indices = malloc(sizeof(uint32_t) * indices_size);
    int index = 0;
    for (int y = 0; y < noise_height - 1; y++)
    {
        for (int x = 0; x < noise_width - 1; x++)
        {
            int yw = y * noise_width;
            indices[index++] = yw + x;
            indices[index++] = yw + noise_width + x;
            indices[index++] = yw + (x + 1);
            indices[index++] = yw + noise_width + x;
            indices[index++] = yw + noise_width + (x + 1);
            indices[index++] = yw + (x + 1);
        }
    }
    Mesh *mesh = Mesh_Create(vertices_size, vertices, indices_size, indices, pivot);
    return mesh;
}