#ifndef COLOR_H
#define COLOR_H

// C
#include <stdint.h>

// ------------------------- 
// Types 
// -------------------------

typedef struct RGBA
{
    float r;
    float g;
    float b;
    float a;
} RGBA;

RGBA RGBA_FromHexaRGBA(uint32_t hexa);

#endif