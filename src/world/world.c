#include "world/world.h"
#include "logging/logger.h"

static LogConfig _logConfig = {"World", LOG_LEVEL_INFO, LOG_COLOR_BLUE};
static World **_worlds = NULL;
static int _worldCount = 0;

World *World_Create(char *name)
{
    World *world = malloc(sizeof(World));
    world->name = name;
    world->rendererCount = 0;
    world->renderers = NULL;
    world->parent = Entity_Create_WorldParent(world, V3_ZERO, 0.0, V2_ONE, V2_HALF);
    _worldCount++;
    _worlds = realloc(_worlds, sizeof(World*) * _worldCount);
    _worlds[_worldCount - 1] = world;
    return world;
}

void World_Free(World *world)
{
    LogFree(&_logConfig, "World<%s>", world->name);
    Entity_Free(world->parent, false);
    free(world->renderers);
    free(world);
}

void World_FreeAll()
{
    LogFree(&_logConfig, "%d Worlds", _worldCount);
    for (int i = 0; i < _worldCount; i++)
    {
        World_Free(_worlds[i]);
    }
    free(_worlds);
}

void World_AddRenderer(EC_Renderer *renderer)
{
    World* world = World_Get(renderer->component->entity);
    world->rendererCount++;
    world->renderers = realloc(world->renderers, sizeof(EC_Renderer) * world->rendererCount);
    world->renderers[world->rendererCount - 1] = renderer;
}

World *World_Get(Entity *entity)
{
    Entity *parent = entity;
    while (parent->parent != NULL)
    {
        parent = parent->parent;
    }
    for (int i = 0; i < _worldCount; i++)
    {
        if (_worlds[i]->parent == parent)
        {
            return _worlds[i];
        }
    }
    return NULL;
}