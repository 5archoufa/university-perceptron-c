#include "rendering/color.h"
// C
#include <stdint.h>

RGBA RGBA_FromHexaRGBA(uint32_t hexa){
    RGBA color={
        ((hexa >> 24) & 0xFF) / 255.0f,
        ((hexa >> 16) & 0xFF) / 255.0f,
        ((hexa >> 8) & 0xFF) / 255.0f,
        (hexa & 0xFF) / 255.0f
    };
    return color;
}