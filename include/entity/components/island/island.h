#ifndef ISLAND_H
#define ISLAND_H

#include "utilities/math/v2.h"
#include "utilities/math/v3.h"
#include "utilities/noise/noise.h"
#include "entity/components/ec_mesh_renderer/ec_mesh_renderer.h"
#include "entity/transform.h"

typedef struct {
    Component *component;
    EC_MeshRenderer* ec_meshRenderer_island;
    Noise* noise;
} EC_Island;

EC_Island* Prefab_Island(Entity* parent, TransformSpace TS, V3 position, Quaternion rotation, V3 scale, V3 meshScale);
V3 EC_Island_GetPositionOnLand(EC_Island* ec_island, V3 position);

#endif