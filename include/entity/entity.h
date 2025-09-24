#ifndef ENTITY_H
#define ENTITY_H

#include "utilities/math/v2.h"
#include "utilities/math/v3.h"

typedef struct Entity Entity;
typedef struct Component Component;

typedef enum
{
    EC_CAMERA,
    EC_RENDERER,
    EC_CIRCLE,
    EC_CAMERA_CONTROLLER,
} EC_Type;

struct Component
{
    void *self;
    Entity *entity;
    EC_Type type;
    void (*Free)(Component *);
    void (*Awake)(Component *);
    void (*Start)(Component *);
    void (*Update)(Component *);
    void (*LateUpdate)(Component *);
};

struct Entity
{
    char *name;
    // Children
    Entity **children;
    int childCount;
    // Transform
    Entity *parent;
    V3 position;
    float rotation;
    V2 scale;
    V2 pivot;
    // Local Transform
    V3 localPos;
    float localRot;
    V2 localScale;
    // Components
    int componentCount;
    EC_Type *_component_keys;
    Component **_component_values;
};

//ENTITIES
void Entity_World_Init();
Entity *Entity_Create_WithoutParent(char* name, V3 position, float rotation, V2 scale, V2 pivot);
void Entity_Free(Entity *entity);
void Entity_Tick(Entity *entity);
Component *Entity_GetComponent(Entity *entity, EC_Type componentType);
void Entity_AddComponent(Entity *entity, Component *component);
void Entity_AddPos(Entity *entity, V3 position);
void Entity_SetPos(Entity *entity, V3 position);
void Entity_World_Init();
void Entity_World_Awake();
void Entity_World_Start();
void Entity_World_Update();
void Entity_World_LateUpdate();
void Entity_World_Free();
// COMPONENTS
Component *Component_Create(void *self,
                            Entity *entity,
                            EC_Type type,
                            void (*Free)(Component *),
                            void (*Awake)(Component *),
                            void (*Start)(Component *),
                            void (*Update)(Component *),
                            void (*LateUpdate)(Component *));
void Component_Free(Component *component);

#endif