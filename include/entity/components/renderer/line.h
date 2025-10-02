#ifndef EC_LINE_H
#define EC_LINE_H

#include "utilities/math/v3.h"
#include "utilities/math/v2.h"
#include "entity/entity.h"
#include "entity/components/renderer/renderer.h"
#include <stdint.h>

typedef struct
{
    EC_Renderer* EC_renderer_line;
    V2 start;
    V2 end;
    float z;
    float width;
    uint32_t pixelColor;
} RD_Line;

EC_Renderer* RD_Line_CreateWithRenderer(Entity* entity, V2 start, V2 end, float z, float width, uint32_t pixelColor);

#endif