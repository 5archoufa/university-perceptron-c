#include "entity/entity.h"
#include "perceptron.h"
// C
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
// Logging
#include "logging/logger.h"
// World
#include "world/world.h"

// -------------------------
// Constants
// -------------------------

static LogConfig _logConfig = {"Entity", LOG_LEVEL_WARN, LOG_COLOR_BLUE};
static uint32_t _nextEntityId = 0;

// -------------------------
// Layers
// -------------------------

void Entity_SetLayer(Entity *entity, uint8_t layer, bool updateChildren)
{
    entity->layer = layer;
    if (updateChildren)
    {
        for (int i = 0; i < entity->transform.children_size; i++)
        {
            Entity_SetLayer(entity->transform.children[i]->entity, layer, true);
        }
    }
}

// -------------------------
// Entity Events
// -------------------------

void Entity_Awake(Entity *entity)
{
    // Log(&_logConfig, "Calling Update() On %d Entities.", _world->childCount);
    Component *component = NULL;
    for (int i = 0; i < entity->componentCount; i++)
    {
        component = entity->component_values[i];
        if (component->Awake != NULL)
        {
            component->Awake(component);
        }
    }
    for (int i = 0; i < entity->transform.children_size; i++)
    {
        Entity_Awake(entity->transform.children[i]->entity);
    }
}

void Entity_Start(Entity *entity)
{
    // Log(&_logConfig, "Calling Update() On %d Entities.", _world->childCount);
    Component *component = NULL;
    for (int i = 0; i < entity->componentCount; i++)
    {
        component = entity->component_values[i];
        if (component->Start != NULL)
        {
            component->Start(component);
        }
    }
    for (int i = 0; i < entity->transform.children_size; i++)
    {
        Entity_Start(entity->transform.children[i]->entity);
    }
}

void Entity_Update(Entity *entity)
{
    // Log(&_logConfig, "Calling Update() On %d Entities.", _world->childCount);
    Component *component = NULL;
    for (int i = 0; i < entity->componentCount; i++)
    {
        component = entity->component_values[i];
        if (component->Update != NULL)
        {
            component->Update(component);
        }
    }
    for (int i = 0; i < entity->transform.children_size; i++)
    {
        Entity_Update(entity->transform.children[i]->entity);
    }
}

void Entity_LateUpdate(Entity *entity)
{
    // Log(&_logConfig, "Calling LateUpdate() On %d Entities.", _world->childCount);
    Component *component = NULL;
    for (int i = 0; i < entity->componentCount; i++)
    {
        component = entity->component_values[i];
        if (component->LateUpdate != NULL)
        {
            component->LateUpdate(component);
        }
    }
    for (int i = 0; i < entity->transform.children_size; i++)
    {
        Entity_LateUpdate(entity->transform.children[i]->entity);
    }
}

void Entity_FixedUpdate(Entity *entity)
{
    // Log(&_logConfig, "Calling LateUpdate() On %d Entities.", _world->childCount);
    Component *component = NULL;
    for (int i = 0; i < entity->componentCount; i++)
    {
        component = entity->component_values[i];
        if (component->FixedUpdate != NULL)
        {
            component->FixedUpdate(component);
        }
    }
    for (int i = 0; i < entity->transform.children_size; i++)
    {
        Entity_FixedUpdate(entity->transform.children[i]->entity);
    }
}

// -------------------------
// Creation
// -------------------------

Component *Component_Create(void *self,
                            Entity *entity,
                            EC_Type type,
                            void (*Free)(Component *),
                            void (*Awake)(Component *),
                            void (*Start)(Component *),
                            void (*Update)(Component *),
                            void (*LateUpdate)(Component *),
                            void (*FixedUpdate)(Component *))
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
    component->FixedUpdate = FixedUpdate;
    Entity_AddComponent(entity, component);
    return component;
}

Entity *Entity_Create(Entity *parent, bool isStatic, char *name, TransformSpace TS, V3 position, Quaternion rotation, V3 scale)
{
    Entity *entity = malloc(sizeof(Entity));
    // Id
    entity->id = _nextEntityId++;
    // Name
    entity->name = name;
    entity->isStatic = isStatic;
    // Layer
    entity->layer = parent->layer;
    // Tag
    entity->tag = E_TAG_NONE;
    // Components
    entity->componentCount = 0;
    entity->component_keys = NULL;
    entity->component_values = NULL;
    // Transform
    Transform_Init(&entity->transform, entity, &parent->transform, TS, position, rotation, scale);
    // Log
    LogCreate(&_logConfig, "%s, Layer %d", name, entity->layer);
    return entity;
}

Entity *Entity_Create_WorldParent(World *world, V3 position, Quaternion rotation, V3 scale)
{
    LogCreate(&_logConfig, "World<%s>'s Parent Entity", world->name);
    Entity *entity = malloc(sizeof(Entity));
    entity->name = world->name;
    entity->isStatic = true;
    // Components
    entity->componentCount = 0;
    entity->component_keys = NULL;
    entity->component_values = NULL;
    // Transform
    Transform_Init(&entity->transform, entity, NULL, TS_WORLD, position, rotation, scale);
    return entity;
}

// -------------------------
// Memory Cleanup
// -------------------------

static void Component_Free(Component *component)
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

void Entity_Free(Entity *entity, bool updateParent)
{
    LogFree(&_logConfig, "%s (%d Children)", entity->name, entity->transform.children_size);
    // Remove from parent
    if (updateParent && entity->transform.parent)
    {
        LogFree(&_logConfig, "Removing %s from its parent %s", entity->name, entity->transform.parent->entity->name);
        Transform_RemoveChild(entity->transform.parent, &entity->transform);
    }
    // Free Components
    printf("[%s]:: Will free %d components...\n", entity->name, entity->componentCount);
    for (int i = 0; i < entity->componentCount; i++)
    {
        Component_Free(entity->component_values[i]);
    }
    free(entity->component_keys);
    free(entity->component_values);
    // Free Children
    for (int i = 0; i < entity->transform.children_size; i++)
    {
        if (entity->transform.children[i] != NULL)
        {
            Entity_Free(entity->transform.children[i]->entity, false);
        }
    }
    free(entity->transform.children);
    // Free Entity
    free(entity);
}

Component *Entity_GetComponent(Entity *entity, EC_Type componentType)
{
    for (int i = 0; i < entity->componentCount; i++)
    {
        if (entity->component_keys[i] == componentType)
        {
            return entity->component_values[i];
        }
    }
    return NULL;
}

void Entity_RemoveComponent(Entity *entity, Component *component)
{
    if (entity->componentCount <= 1)
    {
        entity->componentCount = 0;
        entity->component_keys = NULL;
        entity->component_values = NULL;
        return;
    }
    for (int i = 0; i < entity->componentCount; i++)
    {
        if (entity->component_values[i] == component)
        {
            for (int j = i; j < entity->componentCount - 1; j++)
            {
                entity->component_values[j] = entity->component_values[j + 1];
                entity->component_keys[j] = entity->component_keys[j + 1];
            }
            entity->componentCount--;
            Component **component_values = realloc(entity->component_values, entity->componentCount * sizeof(Component *));
            EC_Type *component_keys = realloc(entity->component_keys, entity->componentCount * sizeof(EC_Type));
            if (component_values != NULL)
            {
                entity->component_values = component_values;
            }
            if (component_keys != NULL)
            {
                entity->component_keys = component_keys;
            }
            break;
        }
    }
}

void Entity_AddComponent(Entity *entity, Component *component)
{
    entity->componentCount++;
    entity->component_values = realloc(entity->component_values, entity->componentCount * sizeof(Component *));
    entity->component_keys = realloc(entity->component_keys, entity->componentCount * sizeof(EC_Type));
    entity->component_keys[entity->componentCount - 1] = component->type;
    entity->component_values[entity->componentCount - 1] = component;
}

// -------------------------
// Active Status
// -------------------------

static void Entity_UpdateActiveState(Entity *entity)
{
    Entity *parent = entity->transform.parent ? entity->transform.parent->entity : NULL;
    if (parent)
    {
        entity->isActive = parent->isActive && entity->isActiveSelf;
    }
    else
    {
        entity->isActive = entity->isActiveSelf;
    }
    // Update components
    for (int i = 0; i < entity->componentCount; i++)
    {
        entity->component_values[i]->isActive = entity->isActive && entity->component_values[i]->isActiveSelf;
    }
    // Update children
    for (int i = 0; i < entity->transform.children_size; i++)
    {
        Entity_UpdateActiveState(entity->transform.children[i]->entity);
    }
}

void Entity_SetActiveSelf(Entity *entity, bool isActiveSelf)
{
    if (entity->isActiveSelf == isActiveSelf)
    {
        return;
    }
    entity->isActiveSelf = isActiveSelf;
    Entity_UpdateActiveState(entity);
}

void Component_SetActiveSelf(Component *component, bool isActiveSelf)
{
    if (component->isActiveSelf == isActiveSelf)
    {
        return;
    }
    component->isActiveSelf = isActiveSelf;
    bool isEntityActive = component->entity->isActive;
    bool newActiveState = isEntityActive && isActiveSelf;
    if (component->isActive != newActiveState)
    {
        component->isActive = newActiveState;
        component->SetActive(component, newActiveState);
    }
}

// -------------------------
// Getters
// -------------------------

inline V3 E_WPos(Entity *entity)
{
    return T_WPos(&entity->transform);
}
inline Quaternion E_WRot(Entity *entity)
{
    return T_WRot(&entity->transform);
}
inline V3 E_WSca(Entity *entity)
{
    return T_WSca(&entity->transform);
}
inline V3 E_LPos(Entity *entity)
{
    return T_LPos(&entity->transform);
}
inline Quaternion E_LRot(Entity *entity)
{
    return T_LRot(&entity->transform);
}
inline void E_LRot_Mul(Entity *entity, Quaternion q)
{
    T_LRot_Mul(&entity->transform, q);
}
inline V3 E_LSca(Entity *entity)
{
    return T_LSca(&entity->transform);
}

inline V3 E_Forward(Entity *entity)
{
    return T_Forward(&entity->transform);
}
inline V3 E_Right(Entity *entity)
{
    return T_Right(&entity->transform);
}
inline V3 E_Up(Entity *entity)
{
    return T_Up(&entity->transform);
}

inline V3 EC_WPos(Component *component)
{
    return T_WPos(&component->entity->transform);
}
inline Quaternion EC_WRot(Component *component)
{
    return T_WRot(&component->entity->transform);
}
inline V3 EC_WSca(Component *component)
{
    return T_WSca(&component->entity->transform);
}

inline V3 EC_LPos(Component *component)
{
    return T_LPos(&component->entity->transform);
}
inline Quaternion EC_LRot(Component *component)
{
    return T_LRot(&component->entity->transform);
}
inline V3 EC_LSca(Component *component)
{
    return T_LSca(&component->entity->transform);
}

inline V3 EC_Forward(Component *component)
{
    return T_Forward(&component->entity->transform);
}
inline V3 EC_Right(Component *component)
{
    return T_Right(&component->entity->transform);
}
inline V3 EC_Up(Component *component)
{
    return T_Up(&component->entity->transform);
}