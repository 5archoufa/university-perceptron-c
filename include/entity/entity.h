#ifndef ENTITY_H
#define ENTITY_H

#include "utilities/math/v2.h"
#include "utilities/math/v3.h"
#include "world/world.h"

typedef struct Entity Entity;
typedef struct Component Component;

typedef enum
{
    /* Cameras */
    EC_T_CAMERA,
    EC_T_CAMERA_CONTROLLER,
    /* Renderers */
    EC_T_RENDERER,
    EC_T_CIRCLE,
    /* Neural Networks */
    EC_T_NN,
    EC_T_NN_LAYER,
    EC_T_NN_NEURON,
    EC_T_NN_LINK,
} EC_Type;

struct Component
{
    void *self;
    Entity *entity;
    EC_Type type;
    bool isActive;
    void (*Free)(Component *);
    void (*Awake)(Component *);
    void (*Start)(Component *);
    void (*Update)(Component *);
    void (*LateUpdate)(Component *);
    void (*FixedUpdate)(Component *);
    void (*SetActive)(Component *, bool isActive);
};

struct Entity
{
    char *name;
    // Children
    Entity **children;
    size_t children_size;
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
    EC_Type *component_keys;
    Component **component_values;
};

//ENTITIES
Entity *Entity_Create(Entity *parent, char *name, V3 position, float rotation, V2 scale, V2 pivot);
Entity *Entity_Create_WorldParent(World* world, V3 position, float rotation, V2 scale, V2 pivot);
void Entity_Free(Entity *entity, bool updateParent);
Component *Entity_GetComponent(Entity *entity, EC_Type componentType);
void Entity_AddComponent(Entity *entity, Component *component);
void Entity_AddPos(Entity *entity, V3 position);
void Entity_SetPos(Entity *entity, V3 position);
void Entity_Awake(Entity *entity);
void Entity_Start(Entity *entity);
void Entity_Update(Entity *entity);
void Entity_LateUpdate(Entity *entity);
void Entity_FixedUpdate(Entity *entity);
// COMPONENTS
Component *Component_Create(void *self,
                            Entity *entity,
                            EC_Type type,
                            void (*Free)(Component *),
                            void (*Awake)(Component *),
                            void (*Start)(Component *),
                            void (*Update)(Component *),
                            void (*LateUpdate)(Component *),
                            void (*FixedUpdate)(Component *));
void Entity_RemoveComponent(Entity *entity, Component *component);
void Component_SetActive(Component* component, bool isActive);

#endif