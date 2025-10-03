#include "utilities/noise/noise.h"
#include "utilities/math/stupid_math.h"
#include "utilities/math/v2.h"

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>

typedef enum
{
    BOTTOM_LEFT,
    BOTTOM_RIGHT,
    TOP_LEFT,
    TOP_RIGHT
} Corner;

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

/// @brief This simply allocates the Noise and configures it. You must call Noise_Generate(Noise*) to get visual results.
Noise *Noise_Create(int width, int height, float interpolation, float min, float max)
{
    // Allocate Noise
    Noise *noise = malloc(sizeof(Noise));
    noise->width = width;
    noise->height = height;
    noise->interp = interpolation;
    noise->min = min;
    noise->max = max;
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
            noise->interpGrid[x][y] = RandomV2(1.0);
        }
    }
    Noise_Update(noise, width, height, interpolation, min, max, false);
    return noise;
}

void Noise_ForceUpdate(Noise *noise)
{
    Noise_Update(noise, noise->width, noise->height, noise->interp, noise->min, noise->max, true);
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
                noise->interpGrid[x][y] = RandomV2(1.0);
                ;
            }
        }
        noise->interpWidth = interpWidth;
        noise->interpHeight = interpHeight;
    }

    if (updateInterpGrid || updateMap)
    {
        // Fill Map with floats
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
    free(noise);
}