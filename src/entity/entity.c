#include <stdlib.h>
#include <stdbool.h>
#include "entity/entity.h"
#include "logging/logger.h"
#include <stdio.h>

static LogConfig _logConfig = {"Entity", LOG_LEVEL_INFO, LOG_COLOR_BLUE};
static Entity *_world;
static bool _isInitialized;

void Entity_World_Init()
{
    if (_isInitialized)
    {
        LogError(&_logConfig, "Can't call Entity_InitWorld(): World is already Initialized.");
        return;
    }
    LogInit(&_logConfig, "");

    // Create World Entity
    LogCreate(&_logConfig, "ENTITIES_ROOT");
    _world = malloc(sizeof(Entity));
    _world->name = "ENTITIES_ROOT";
    // Children
    _world->children = NULL;
    _world->childCount = 0;
    // Transform
    _world->parent = NULL;
    _world->position = V3_ZERO;
    _world->rotation = 0.0;
    _world->localPos = V3_ZERO;
    _world->scale = V2_ONE;
    _world->localScale = V2_ONE;
    _world->pivot = V2_HALF;
    _world->localRot = 0.0;
    // Components
    _world->componentCount = 0;
    _world->_component_keys = NULL;
    _world->_component_values = NULL;

    _isInitialized = true;
}

void Entity_World_Awake()
{
    // Log(&_logConfig, "Calling Awake() On %d Entities.", _world->childCount);
    Entity *entity = NULL;
    Component *component = NULL;
    for (int e = 0; e < _world->childCount; e++)
    {
        entity = _world->children[e];
        for (int c = 0; c < entity->componentCount; c++)
        {
            component = entity->_component_values[c];
            if (component->Awake != NULL)
            {
                component->Awake(component);
            }
        }
    }
}

void Entity_World_Start()
{
    // Log(&_logConfig, "Calling Start() On %d Entities.", _world->childCount);
    Entity *entity = NULL;
    Component *component = NULL;
    for (int e = 0; e < _world->childCount; e++)
    {
        entity = _world->children[e];
        for (int c = 0; c < entity->componentCount; c++)
        {
            component = entity->_component_values[c];
            if (component->Start != NULL)
            {
                component->Start(component);
            }
        }
    }
}

void Entity_World_Update()
{
    // Log(&_logConfig, "Calling Update() On %d Entities.", _world->childCount);
    Entity *entity = NULL;
    Component *component = NULL;
    for (int e = 0; e < _world->childCount; e++)
    {
        entity = _world->children[e];
        for (int c = 0; c < entity->componentCount; c++)
        {
            component = entity->_component_values[c];
            if (component->Update != NULL)
            {
                component->Update(component);
            }
        }
    }
}

void Entity_World_LateUpdate()
{
    // Log(&_logConfig, "Calling LateUpdate() On %d Entities.", _world->childCount);
    Entity *entity = NULL;
    Component *component = NULL;
    for (int e = 0; e < _world->childCount; e++)
    {
        entity = _world->children[e];
        for (int c = 0; c < entity->componentCount; c++)
        {
            component = entity->_component_values[c];
            if (component->LateUpdate != NULL)
            {
                component->LateUpdate(component);
            }
        }
    }
}

void Entity_World_Free()
{
    Entity_Free(_world);
}

static void Entity_AddChild(Entity *entity, Entity *child)
{
    entity->childCount++;
    if (entity->children == NULL)
    {
        entity->children = malloc(sizeof(Entity *));
    }
    else
    {
        entity->children = realloc(entity->children, entity->childCount * sizeof(Entity *));
    }
    entity->children[entity->childCount - 1] = child;
}

Entity *Entity_Create_WithoutParent(char *name, V3 position, float rotation, V2 scale, V2 pivot)
{
    LogCreate(&_logConfig, name);
    Entity *entity = malloc(sizeof(Entity));
    entity->name = name;
    // Children
    entity->children = NULL;
    entity->childCount = 0;
    // Transform
    entity->parent = _world;
    entity->position = position;
    entity->rotation = rotation;
    entity->scale = _world->scale;
    entity->pivot = pivot;
    // Local Transform
    entity->localPos = V3_ZERO;
    entity->localRot = 0.0;
    entity->localScale = V2_ONE;
    // Components
    entity->componentCount = 0;
    entity->_component_keys = NULL;
    entity->_component_values = NULL;
    Entity_AddChild(_world, entity);

    return entity;
}

void Entity_Free(Entity *entity)
{
    LogFree(&_logConfig, "%s (%d Children)", entity->name, entity->childCount);
    for (int i = 0; i < entity->componentCount; i++)
    {
        printf("Will free component[%d]\n", i);
        Component_Free(entity->_component_values[i]);
    }
    free(entity->_component_keys);
    free(entity->_component_values);
    for (int i = 0; i < entity->childCount; i++)
    {
        if (entity->children[i] != NULL)
        {
            Entity_Free(entity->children[i]);
        }
    }
    free(entity->children);
    free(entity);
}

Component *Entity_GetComponent(Entity *entity, EC_Type componentType)
{
    for (int i = 0; i < entity->componentCount; i++)
    {
        if (entity->_component_keys[i] == componentType)
        {
            return entity->_component_values[i];
        }
    }
    return NULL;
}

void Entity_AddComponent(Entity *entity, Component *component)
{
    entity->componentCount++;
    if (entity->_component_values == NULL)
    {
        entity->_component_values = malloc(sizeof(Component *));
    }
    else
    {
        entity->_component_values = realloc(entity->_component_values, entity->componentCount * sizeof(Component *));
    }
    if (entity->_component_keys == NULL)
    {
        entity->_component_keys = malloc(sizeof(EC_Type));
    }
    else
    {
        entity->_component_keys = realloc(entity->_component_keys, entity->componentCount * sizeof(EC_Type));
    }
    entity->_component_keys[entity->componentCount-1] = component->type;
    entity->_component_values[entity->componentCount - 1] = component;
}

void Entity_AddLocalPos(Entity *entity, V3 addition)
{
    Entity_SetPos(entity, V3_ADD(&entity->position, &addition));
}

void Entity_SetLocalPos(Entity *entity, V3 localPos)
{
    V3 diff = V3_SUB(&localPos, &entity->localPos);
    Entity_SetPos(entity, V3_ADD(&entity->position, &diff));
}

void Entity_AddPos(Entity *entity, V3 addition)
{
    Entity_SetPos(entity, V3_ADD(&entity->position, &addition));
}

void Entity_SetParent(Entity *entity, Entity *parent)
{
    // Update local pos
    entity->parent = parent;
    entity->localPos = V3_SUB(&entity->position, &parent->position);
}

void Entity_SetPos(Entity *entity, V3 position)
{
    // Update local position
    V3 diff = V3_SUB(&position, &entity->position);
    if (entity->parent != NULL)
    {
        V3_ADD_FIRST(&entity->localPos, &diff);
    }
    for (int i = 0; i < entity->childCount; i++)
    {
        Entity_AddPos(entity->children[i], diff);
    }
    // Update global position
    entity->position = position;
}

// COMPONENTS
Component *Component_Create(void *self,
                            Entity *entity,
                            EC_Type type,
                            void (*Free)(Component *),
                            void (*Awake)(Component *),
                            void (*Start)(Component *),
                            void (*Update)(Component *),
                            void (*LateUpdate)(Component *))
{
    Component *component = malloc(sizeof(Component));
    component->entity = entity;
    component->self = self;
    component->type = type;
    component->Free = Free;
    component->Awake = Awake;
    component->Start = Start;
    component->Update = Update;
    component->LateUpdate = LateUpdate;
    Entity_AddComponent(entity, component);
    return component;
}

void Component_Free(Component *component)
{
    if (component != NULL)
    {
        component->Free(component);
    }
    else
    {
        LogWarning(&_logConfig, "Couldn't call Free() on component.self (NULL). Will Ignore.");
    }
    free(component);
}