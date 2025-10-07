#include "world/world.h"
#include "logging/logger.h"
#include "entity/components/lighting/ec_light.h"

static LogConfig _logConfig = {"World", LOG_LEVEL_INFO, LOG_COLOR_BLUE};
static World **_worlds = NULL;
static int _worldCount = 0;

// -------------------------
// Creation and Freeing
// -------------------------

World *World_Create(char *name)
{
    World *world = malloc(sizeof(World));
    // Name
    world->name = name;
    // Renderers
    world->rendererCount = 0;
    world->renderers = NULL;
    world->rendererCount_3D = 0;
    world->renderers_3D = NULL;
    // Directional Lights
    world->lights_directional_size = 0;
    world->lights_directional = NULL;
    // Point Lights
    world->lights_point_size = 0;
    world->lights_point = NULL;
    // Entity
    world->parent = Entity_Create_WorldParent(world, V3_ZERO, QUATERNION_IDENTITY, V3_ONE);
    // Update Worlds container
    _worldCount++;
    _worlds = realloc(_worlds, sizeof(World *) * _worldCount);
    _worlds[_worldCount - 1] = world;
    return world;
}

void World_Free(World *world, bool resizeWorlds)
{
    LogFree(&_logConfig, "World<%s>, Resize List of Worlds<%d>", world->name, resizeWorlds);
    // Update world count
    for (int i = 0; i < _worldCount; i++)
    {
        if (_worlds[i] == world)
        {
            _worldCount--;
            if (resizeWorlds) // Resize Worlds container
            {
                // Shift remaining worlds down
                for (int j = i; j < _worldCount - 1; j++)
                {
                    _worlds[j] = _worlds[j + 1];
                }
                _worlds = realloc(_worlds, sizeof(World *) * _worldCount);
            }
            break;
        }
    }
    // Free Entities
    Entity_Free(world->parent, false);
    // Free struct
    free(world->renderers);
    free(world->renderers_3D);
    free(world->lights_directional);
    free(world);
}

void World_All_Free()
{
    LogFree(&_logConfig, "%d Worlds", _worldCount);
    for (int i = 0; i < _worldCount; i++)
    {
        World_Free(_worlds[i], false);
    }
    free(_worlds);
}

// -------------------------
// Lighting
// -------------------------

void World_Light_Add(EC_Light *ec_light)
{
    Log(&_logConfig, "Adding light to world from entity '%s'", ec_light->component->entity->name);
    World *world = World_Get(ec_light->component->entity);
    if (!world)
    {
        LogFree(&_logConfig, "Failed to add light to world, entity '%s' has no world", ec_light->component->entity->name);
        return;
    }

    switch (ec_light->type)
    {
    case LS_T_DIRECTIONAL:
        world->lights_directional_size++;
        world->lights_directional = realloc(world->lights_directional, sizeof(EC_Light *) * world->lights_directional_size);
        world->lights_directional[world->lights_directional_size - 1] = ec_light;
        break;
    case LS_T_POINT:
        world->lights_point_size++;
        world->lights_point = realloc(world->lights_point, sizeof(EC_Light *) * world->lights_point_size);
        world->lights_point[world->lights_point_size - 1] = ec_light;
        break;
    default:
        LogError(&_logConfig, "A light source (%s) of an unknown type has been added to the world. It will not be buffered nor considered in rendering.", ec_light->component->entity->name);
        break;
    }
}

void World_Light_Remove(EC_Light *ec_light)
{
    World *world = World_Get(ec_light->component->entity);
    if (!world)
    {
        LogFree(&_logConfig, "Failed to remove light from world, entity '%s' has no world", ec_light->component->entity->name);
        return;
    }
    switch (ec_light->type)
    {
    case LS_T_DIRECTIONAL:
        for (int i = 0; i < world->lights_directional_size; i++)
        {
            if (world->lights_directional[i] == ec_light)
            {
                world->lights_directional_size--;
                // Shift remaining lights down
                for (int j = i; j < world->lights_directional_size; j++)
                {
                    world->lights_directional[j] = world->lights_directional[j + 1];
                }
                world->lights_directional = realloc(world->lights_directional, sizeof(EC_Light *) * world->lights_directional_size);
                break;
            }
        }
        break;
    case LS_T_POINT:
        for (int i = 0; i < world->lights_point_size; i++)
        {
            if (world->lights_point[i] == ec_light)
            {
                world->lights_point_size--;
                // Shift remaining lights down
                for (int j = i; j < world->lights_point_size; j++)
                {
                    world->lights_point[j] = world->lights_point[j + 1];
                }
                world->lights_point = realloc(world->lights_point, sizeof(EC_Light *) * world->lights_point_size);
                break;
            }
        }
        break;
    default:
        break;
    }
}

// -------------------------
// 3D Renderers
// -------------------------

void World_Renderer3D_Add(EC_Renderer3D *ec_renderer3D)
{
    World *world = World_Get(ec_renderer3D->component->entity);
    world->rendererCount_3D++;
    world->renderers_3D = realloc(world->renderers_3D, sizeof(EC_Renderer3D) * world->rendererCount_3D);
    world->renderers_3D[world->rendererCount_3D - 1] = ec_renderer3D;
    printf("Added 3D Renderer to World '%s', total 3D renderers: %d\n", world->name, world->rendererCount_3D);
}

void World_Renderer3D_Remove(EC_Renderer3D *ec_renderer3D)
{
    World *world = World_Get(ec_renderer3D->component->entity);
    for (int i = 0; i < world->rendererCount_3D; i++)
    {
        if (world->renderers_3D[i] == ec_renderer3D)
        {
            world->rendererCount_3D--;
            // Shift remaining renderers down
            for (int j = i; j < world->rendererCount_3D; j++)
            {
                world->renderers_3D[j] = world->renderers_3D[j + 1];
            }
            world->renderers_3D = realloc(world->renderers_3D, sizeof(EC_Renderer3D) * world->rendererCount_3D);
            break;
        }
    }
}

// -------------------------
// 2D Renderers
// -------------------------

void World_Renderer_Add(EC_Renderer *renderer)
{
    World *world = World_Get(renderer->component->entity);
    world->rendererCount++;
    world->renderers = realloc(world->renderers, sizeof(EC_Renderer) * world->rendererCount);
    world->renderers[world->rendererCount - 1] = renderer;
}

void World_Renderer_Remove(EC_Renderer *renderer)
{
    World *world = World_Get(renderer->component->entity);
    for (int i = 0; i < world->rendererCount; i++)
    {
        if (world->renderers[i] == renderer)
        {
            world->rendererCount--;
            // Shift remaining renderers down
            for (int j = i; j < world->rendererCount; j++)
            {
                world->renderers[j] = world->renderers[j + 1];
            }
            world->renderers = realloc(world->renderers, sizeof(EC_Renderer) * world->rendererCount);
            break;
        }
    }
}

// -------------------------
// World Management
// -------------------------

World *World_Get(Entity *entity)
{
    Transform *parentT = entity->transform.parent;
    while (parentT->parent != NULL)
    {
        parentT = parentT->parent;
    }
    for (int i = 0; i < _worldCount; i++)
    {
        if (_worlds[i]->parent == parentT->entity)
        {
            return _worlds[i];
        }
    }
    return NULL;
}