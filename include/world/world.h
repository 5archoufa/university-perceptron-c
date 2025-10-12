#ifndef WORLD_H
#define WORLD_H

// C
#include <stdlib.h>
#include <stdint.h>
// Entity
#include "entity/entity.h"
// Renderers
#include "entity/components/renderer/renderer.h"
#include "entity/components/ec_renderer3d/ec_renderer3d.h"
// Material
#include "rendering/material/material-manager.h"
// Lighting
#include "entity/components/lighting/ec_light.h"
// Physics
#include "physics/physics-manager.h"

typedef struct World World;
typedef struct EC_Light EC_Light;
typedef struct EC_Renderer3D EC_Renderer3D;
typedef struct PhysicsManager PhysicsManager;

struct World
{
    char *name;
    // ============ Entities ============ //
    Entity *parent;
    // ============ Physics ============ //
    PhysicsManager *physicsManager;
    // ============ Renderers ============ //
    int rendererCount;
    EC_Renderer **renderers;
    // 3D Renderers
    int rendererCount_3D;
    EC_Renderer3D **renderers_3D;
    // ============ Lighting ============ //
    // Directional Lights
    size_t lights_directional_size;
    EC_Light **lights_directional;
    // Point Lights
    size_t lights_point_size;
    EC_Light **lights_point;
    // ============ Tracking State ============ //
    /// @brief Whether or not this world is marked for freeing
    bool isMarkedForFreeing;
};

// -------------------------
// Creation and Freeing
// -------------------------

World *World_Create(char *name);
void World_Free(World *world, bool resizeWorlds);
void World_All_Free();

// -------------------------
// 3D Renderers
// -------------------------

void World_Renderer3D_Add(EC_Renderer3D *ec_renderer3D);
void World_Renderer3D_Remove(EC_Renderer3D *ec_renderer3D);

// -------------------------
// 2D Renderers
// -------------------------

void World_Renderer_Add(EC_Renderer *ec_renderer);
void World_Renderer_Remove(EC_Renderer *ec_renderer);

// -------------------------
// Lighting
// -------------------------

void World_Light_Add(EC_Light *ec_light);
void World_Light_Remove(EC_Light *ec_light);

// -------------------------
// World Management
// -------------------------

World *World_Get(Entity *entity);

#endif