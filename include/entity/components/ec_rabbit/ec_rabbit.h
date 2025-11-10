#ifndef EC_RABBIT_H
#define EC_RABBIT_H

// Entity
#include "entity/entity.h"
// Creature
#include "entity/components/ec_creature/ec_creature.h"

// ------------------------- 
// Types 
// -------------------------

typedef struct EC_Rabbit
{
    Component *component;
    EC_Creature *creature;
} EC_Rabbit;
// ------------------------- 
// Creation & Freeing 
// -------------------------

EC_Rabbit *EC_Rabbit_Create(EC_Island *ec_island, Entity *e_rabbit);

#endif