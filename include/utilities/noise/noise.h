#ifndef NOISE_H
#define NOISE_H

#include "utilities/math/v2.h"
#include <stdbool.h>
#include "rendering/mesh/mesh.h"
#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stddef.h> // for offsetof
#include "rendering/texture/texture.h"
// OpenGL
#include <glad/glad.h>

typedef struct Noise Noise;

typedef struct {
    void (*function)(Noise *noise, int argCount, void** args);
    int argCount;
    void** args;
} NoiseModifier;

struct Noise
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
    size_t modifiers_size;
    NoiseModifier *modifiers;
};

typedef struct
{
    V2 yLevel;
    uint32_t color;
} NoiseLayer;

Noise *Noise_Create(int width, int height, float interpolation, float min, float max, size_t modifiers_size, NoiseModifier* modifiers);
void Noise_Free(Noise *noise);
void Noise_Update(Noise *noise, int width, int height, float interpolation, float min, float max, bool forceUpdate);
void Noise_Regenerate(Noise *noise);
void Noise_RecalculateMap(Noise *noise);
Mesh *Noise_CreateMesh(Noise *noise, V3 meshScale, int layers_size, const NoiseLayer *layers, bool usePixelColors, Texture* texture, int density, V3 pivot);
void Noise_Modifier_Mask_Circle(Noise *noise, int argCount, void** args);
Mesh *Mesh_CreatePlane(V2 meshScale, V2_INT vertexCount, uint32_t color, V2 pivot);
void Noise_AddModifier(Noise *noise, NoiseModifier modifier);
#endif