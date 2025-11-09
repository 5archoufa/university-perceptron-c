#ifndef EC_CREATURE_H
#define EC_CREATURE_H

// Entity
#include "entity/entity.h"
#include "entity/transform.h"
// Island
#include "entity/components/island/island.h"

// -------------------------
// Types
// -------------------------

typedef struct EC_Creature EC_Creature;

typedef enum
{
    CREATURE_T_HUMAN,
    CREATURE_T_SHEEP,
    CREATURE_T_TRIGONOID
} CreatureType;

typedef struct CreatureController
{
    EC_Creature *ec_creature;
    // Movement
    V2 movementSpeed;
    V2 lookSpeed;
    // Input
    V2 input_move;
    V2 input_look;
    // Grounding
    RaycastHit groundHit;
} CreatureController;

struct EC_Creature
{
    Component *component;
    CreatureType type;
    CreatureController controller;
    // References
    Transform *transform;
    EC_MeshRenderer *ec_meshRenderer;
    EC_Island *ec_island;
    // Caching
    V3 previousPos;
};

// -------------------------
// Creation & Freeing
// -------------------------

EC_Creature *EC_Creature_Create(EC_Island *ec_island, Entity *entity, CreatureType type, EC_MeshRenderer *ec_meshRenderer, V2 movementSpeed, V2 lookSpeed);

#endif