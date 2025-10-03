#ifndef NOISE_H
#define NOISE_H

#include "utilities/math/v2.h"
#include <stdbool.h>

typedef struct
{
    int width;
    int height;
    int interp;
    float **map;
    V2 **interpGrid;
    int interpWidth;
    int interpHeight;
    float min;
    float max;
} Noise;

Noise *Noise_Create(int width, int height, float interpolation, float min, float max);
void Noise_Free(Noise *noise);
void Noise_Update(Noise *noise, int width, int height, float interpolation, float min, float max, bool forceUpdate);
void Noise_ForceUpdate(Noise *noise);
#endif