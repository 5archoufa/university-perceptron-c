#ifndef EC_HUMAN_H
#define EC_HUMAN_H

// Entity
#include "entity/entity.h"
// Creature
#include "entity/components/ec_creature/ec_creature.h"

// ------------------------- 
// Types 
// -------------------------

typedef struct EC_Human
{
    Component *component;
    EC_Creature *creature;
} EC_Human;

// ------------------------- 
// Creation & Freeing 
// -------------------------

EC_Human *EC_Human_Create(EC_Island *ec_island, Entity *e_human);

#endif