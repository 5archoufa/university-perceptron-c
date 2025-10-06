#ifndef ISLAND_H
#define ISLAND_H

#include "utilities/math/v2.h"
#include "utilities/math/v3.h"
#include "utilities/noise/noise.h"
#include "entity/components/ec_renderer3d/ec_renderer3d.h"
#include "input/input_manager.h"
#include "entity/transform.h"

typedef struct {
    Component *component;
    EC_Renderer3D* ec_renderer3d_island;
    InputContext* inputContext;
} EC_Island;

EC_Island* Prefab_Island(Entity* parent, TransformSpace TS, V3 position, Quaternion rotation, V3 scale, V3 meshScale);

#endif