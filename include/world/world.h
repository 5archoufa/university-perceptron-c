#ifndef WORLD_H
#define WORLD_H

// C
#include <stdlib.h>
#include <stdint.h>
// Entity
#include "entity/entity.h"
// Renderers
#include "entity/components/ec_mesh_renderer/ec_mesh_renderer.h"
// Material
#include "rendering/material/material-manager.h"
// Lighting
#include "entity/components/lighting/ec_light.h"
// Physics
#include "physics/physics-manager.h"
// GUI
#include "entity/components/gui/ec_gui.h"

typedef struct World World;
typedef struct EC_Light EC_Light;
typedef struct EC_MeshRenderer EC_MeshRenderer;
typedef struct PhysicsManager PhysicsManager;
typedef struct EC_GUI EC_GUI;

typedef struct SimulatedDateTime
{
    // ============ Date ============ // 
    /// @brief Day of the month (1 - 31)
    int day;
    /// @brief Normalized day (0.0 - 1.0)
    float normalizedDay;
    /// @brief Month (1 - 12)
    int month;
    /// @brief Normalized month (0.0 - 1.0)
    float normalizedMonth;
    /// @brief Year (0 - 9999)
    int year;
    // ============ Time ============ // 
    int hours;
    /// @brief Normalized hours (0.0 - 1.0)
    float normalizedHours;
    /// @brief Minutes (0 - 60)
    int minutes;
    /// @brief Normalized minutes (0.0 - 1.0)
    float normalizedMinutes;
    /// @brief Seconds (0 - 60)
    float seconds;
    /// @brief Normalized seconds (0.0 - 1.0)
    float normalizedSeconds;
} SimulatedDateTime;

struct World
{
    char *name;
    // ============ Time ============ // 
    SimulatedDateTime dateTime;
    float simulationSecondScalor;
    // ============ Skybox ============ //
    Material *skyboxMaterial;
    Mesh *skyboxMesh;
    // ============ Entities ============ //
    Entity *parent;
    // ============ Physics ============ //
    PhysicsManager *physicsManager;
    // ============ Renderers ============ //
    // Mesh Renderers
    int meshRenderers_size;
    EC_MeshRenderer **meshRenderers;
    // ============ Lighting ============ //
    // Directional Lights
    size_t lights_directional_size;
    EC_Light **lights_directional;
    // Point Lights
    size_t lights_point_size;
    EC_Light **lights_point;
    // ============ GUI ============ //
    size_t guis_size;
    EC_GUI **guis;
    // ============ Tracking State ============ //
    /// @brief Whether or not this world is marked for freeing
    bool isMarkedForFreeing;
};

// ------------------------- 
// Worlds Management 
// -------------------------

Entity *World_GetRoot();
void World_Select(World *world);

// ------------------------- 
// Time Utilities 
// -------------------------

SimulatedDateTime *World_GetDateTime();
size_t *DateTime_GetStringBufferSize();
void DateTime_ToString(SimulatedDateTime *dt, char *buffer, size_t bufferSize);

// ------------------------- 
// Entity Events
// -------------------------

void World_Update();

// -------------------------
// Creation and Freeing
// -------------------------

World *World_Create(char *name, float simulationSecondScalor);
void World_Free(World *world, bool resizeWorlds);
void World_All_Free();

// -------------------------
// 3D Renderers
// -------------------------

void World_Renderer3D_Add(EC_MeshRenderer *ec_meshRenderer);
void World_Renderer3D_Remove(EC_MeshRenderer *ec_meshRenderer);

// -------------------------
// Lighting
// -------------------------

void World_Light_Add(EC_Light *ec_light);
void World_Light_Remove(EC_Light *ec_light);

// -------------------------
// GUI
// -------------------------

void World_GUI_Add(EC_GUI *ec_gui);
void World_GUI_Remove(EC_GUI *ec_gui);

// -------------------------
// World Management
// -------------------------

World *World_Get(Entity *entity);

#endif