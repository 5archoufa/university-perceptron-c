#ifndef ENTITY_H
#define ENTITY_H

#include "utilities/math/v2.h"
#include "utilities/math/v3.h"
#include "world/world.h"
#include "entity/transform.h"

// -------------------------
// Type Definitions
// -------------------------

typedef struct Entity Entity;
typedef struct Component Component;
typedef struct Transform Transform;
typedef struct Quaternion Quaternion;
typedef enum TransformSpace TransformSpace;

typedef enum
{
    /* Cameras */
    EC_T_CAMERA,
    EC_T_CAMERA_CONTROLLER,
    /* Renderers */
    EC_T_RENDERER,
    EC_T_CIRCLE,
    EC_T_RENDERER3D,
    /* Neural Networks */
    EC_T_NN,
    EC_T_NN_LAYER,
    EC_T_NN_NEURON,
    EC_T_NN_LINK,
    /* Island */
    EC_T_ISLAND,
    /* Water */
    EC_T_WATER,
    /* Lighting */
    EC_T_LIGHT,
    /* Sky */
    EC_T_PLANET,
    /* Creatures */
    EC_T_CREATURE,
    EC_T_HUMAN,
    /* Player */
    EC_T_PLAYER,
} EC_Type;

struct Component
{
    void *self;
    Entity *entity;
    EC_Type type;
    bool isActiveSelf;
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
    bool isActiveSelf;
    bool isActive;
    // Transform
    Transform transform;
    // Components
    int componentCount;
    EC_Type *component_keys;
    Component **component_values;
};

// -------------------------
// Creation & Freeing
// -------------------------

Entity *Entity_Create(Entity *parent,  char *name, TransformSpace TS, V3 position, Quaternion rotation, V3 scale);
Entity *Entity_Create_WorldParent(World* world, V3 position, Quaternion rotation, V3 scale);
void Entity_Free(Entity *entity, bool updateParent);
Component *Component_Create(void *self,
                            Entity *entity,
                            EC_Type type,
                            void (*Free)(Component *),
                            void (*Awake)(Component *),
                            void (*Start)(Component *),
                            void (*Update)(Component *),
                            void (*LateUpdate)(Component *),
                            void (*FixedUpdate)(Component *));

// -------------------------
// Entity Management
// -------------------------

void Component_SetActive(Component* component, bool isActive);

// -------------------------
// Component Management
// -------------------------

void Entity_AddComponent(Entity *entity, Component *component);
void Entity_RemoveComponent(Entity *entity, Component *component);
Component *Entity_GetComponent(Entity *entity, EC_Type componentType);

// -------------------------
// Entity Events
// -------------------------

void Entity_Awake(Entity *entity);
void Entity_Start(Entity *entity);
void Entity_Update(Entity *entity);
void Entity_LateUpdate(Entity *entity);
void Entity_FixedUpdate(Entity *entity);

// -------------------------
// Getters
// -------------------------

V3 E_WPos(Entity *entity);
Quaternion E_WRot(Entity *entity);
V3 E_WSca(Entity *entity);

V3 E_LPos(Entity *entity);
Quaternion E_LRot(Entity *entity);
V3 E_LSca(Entity *entity);

V3 E_Forward(Entity *entity);
V3 E_Right(Entity *entity);
V3 E_Up(Entity *entity);

V3 EC_WPos(Component *component);
Quaternion EC_WRot(Component *component);
V3 EC_WSca(Component *component);

V3 EC_LPos(Component *component);
Quaternion EC_LRot(Component *component);
V3 EC_LSca(Component *component);

V3 EC_Forward(Component *component);
V3 EC_Right(Component *component);
V3 EC_Up(Component *component);

#endif