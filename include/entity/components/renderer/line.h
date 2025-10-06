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
    V3 start;
    V3 end;
    float thickness;
    uint32_t color;
} RD_Line;

EC_Renderer* RD_Line_CreateWithRenderer(Entity* entity, V3 start, V3 end, float thickness, uint32_t color);

#endif