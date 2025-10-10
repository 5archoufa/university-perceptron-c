#ifndef EC_PLAYER_H
#define EC_PLAYER_H

// Entity
#include "entity/entity.h"
// Human
#include "entity/components/ec_human/ec_human.h"
// Input
#include "input/input_manager.h"

// ------------------------- 
// Types 
// -------------------------

typedef struct EC_Player{
    Component *component;
    // Human
    EC_Human *human;
    // Input
    InputContext *inputContext;
} EC_Player;

// ------------------------- 
// Creation & Freeing 
// -------------------------

EC_Player *EC_Player_Prefab(Entity* parent, TransformSpace TS, V3 position, Quaternion rotation, V3 scale);

#endif