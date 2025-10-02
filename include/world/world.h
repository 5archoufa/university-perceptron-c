#ifndef WORLD_H
#define WORLD_H

#include <stdlib.h>
#include "entity/components/renderer/renderer.h"
#include "entity/entity.h"

typedef struct World World;
typedef struct EC_Renderer EC_Renderer;

struct World
{
    char* name;
    Entity* parent;
    int rendererCount;
    EC_Renderer **renderers;
};

World* World_Create(char* name);
void World_Free(World* world);
void World_AddRenderer(EC_Renderer* renderer);
World* World_Get(Entity* entity);
void World_FreeAll();
#endif