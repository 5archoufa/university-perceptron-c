#ifndef ISLAND_H
#define ISLAND_H

#include "utilities/math/v2.h"
#include "utilities/math/v3.h"
#include "utilities/noise/noise.h"
#include "entity/components/renderer/renderer.h"
#include "input/input_manager.h"

typedef struct {
    Component *component;
    EC_Renderer* ec_renderer_island;
    InputContext* inputContext;
} EC_Island;

EC_Island* Prefab_Island(Entity* parent, V3 position, float rotation, V2 scale, V2 pivot);

#endif