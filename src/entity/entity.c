#include <stdlib.h>
#include <stdbool.h>
#include "entity/entity.h"
#include "logging/logger.h"
#include "world/world.h"
#include <stdio.h>

static LogConfig _logConfig = {"Entity", LOG_LEVEL_INFO, LOG_COLOR_BLUE};

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
    for (int i = 0; i < entity->children_size; i++)
    {
        Entity_Awake(entity->children[i]);
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
    for (int i = 0; i < entity->children_size; i++)
    {
        Entity_Start(entity->children[i]);
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
    for (int i = 0; i < entity->children_size; i++)
    {
        Entity_Update(entity->children[i]);
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
    for (int i = 0; i < entity->children_size; i++)
    {
        Entity_LateUpdate(entity->children[i]);
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
    for (int i = 0; i < entity->children_size; i++)
    {
        Entity_FixedUpdate(entity->children[i]);
    }
}

static void Entity_SetParent(Entity *parent, Entity *child)
{
    child->parent = parent;
    // Update local transform
    child->localPos = V3_SUB(&child->position, &parent->position);
    child->localRot = child->rotation - parent->rotation;
    child->localScale = V2_DIV(&child->scale, &parent->scale);
    // Update parent
    parent->children_size++;
    parent->children = realloc(parent->children, parent->children_size * sizeof(Entity *));
    parent->children[parent->children_size - 1] = child;
}

Entity *Entity_Create(Entity *parent, char *name, V3 position, float rotation, V2 scale, V2 pivot)
{
    LogCreate(&_logConfig, name);
    Entity *entity = malloc(sizeof(Entity));
    entity->name = name;
    // Transform
    entity->position = position;
    entity->rotation = rotation;
    entity->scale = scale;
    entity->pivot = pivot;
    // Components
    entity->componentCount = 0;
    entity->component_keys = NULL;
    entity->component_values = NULL;
    // Children
    entity->children = NULL;
    entity->children_size = 0;
    Entity_SetParent(parent, entity);

    return entity;
}

Entity *Entity_Create_WorldParent(World *world, V3 position, float rotation, V2 scale, V2 pivot)
{
    LogCreate(&_logConfig, "World<%s>'s Parent Entity", world->name);
    Entity *entity = malloc(sizeof(Entity));
    entity->name = world->name;
    // Transform
    entity->parent = NULL;
    entity->position = position;
    entity->rotation = rotation;
    entity->scale = scale;
    entity->pivot = pivot;
    // Local Transform
    entity->localPos = V3_ZERO;
    entity->localRot = 0.0;
    entity->localScale = V2_ONE;
    // Components
    entity->componentCount = 0;
    entity->component_keys = NULL;
    entity->component_values = NULL;
    // Children
    entity->children = NULL;
    entity->children_size = 0;

    return entity;
}

static void Entity_RemoveChild(Entity *parent, Entity *child)
{
    if (parent->children_size <= 1)
    {
        parent->children_size = 0;
        parent->children = NULL;
        return;
    }
    for (int i = 0; i < parent->children_size; i++)
    {
        if (parent->children[i] == child)
        {
            for (int j = i; j < parent->children_size - 1; j++)
            {
                parent->children[j] = parent->children[j + 1];
            }
            parent->children_size--;
            Entity **children = realloc(parent->children, parent->children_size * sizeof(Entity *));
            if (children != NULL)
            {
                parent->children = children;
            }
            break;
        }
    }
}

void Entity_Free(Entity *entity, bool updateParent)
{
    LogFree(&_logConfig, "%s (%d Children)", entity->name, entity->children_size);
    // Remove from parent
    if (updateParent && entity->parent != NULL)
    {
        LogFree(&_logConfig, "Removing %s from its parent %s", entity->name, entity->parent->name);
        Entity_RemoveChild(entity->parent, entity);
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
    for (int i = 0; i < entity->children_size; i++)
    {
        if (entity->children[i] != NULL)
        {
            Entity_Free(entity->children[i], false);
        }
    }
    free(entity->children);
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

void Entity_SetPos(Entity *entity, V3 position)
{
    // Update local position
    V3 diff = V3_SUB(&position, &entity->position);
    if (entity->parent != NULL)
    {
        V3_ADD_FIRST(&entity->localPos, &diff);
    }
    for (int i = 0; i < entity->children_size; i++)
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

void Component_SetActive(Component *component, bool isActive)
{
    if (component->isActive == isActive)
    {
        return;
    }
    component->isActive = isActive;
    component->SetActive(component, isActive);
}