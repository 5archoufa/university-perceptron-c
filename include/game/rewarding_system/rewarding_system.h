#ifndef REWARDING_SYSTEM_H
#define REWARDING_SYSTEM_H

// Feelings
#include "game/creature/feelings.h"
// Personality
#include "game/creature/personality.h"

// ----------------------------------------
// Types
// ----------------------------------------

typedef struct RewardingSystem
{
    
} RewardingSystem;

// ----------------------------------------
// Creation & Freeing
// ----------------------------------------

RewardingSystem *RewardingSystem_Create();
void RewardingSystem_Free(RewardingSystem *system);

// ----------------------------------------
// API
// ----------------------------------------

void RewardingSystem_RewardEmotions(RewardingSystem *system, EmotionalFeelings *feelings, Personality *personality);

#endif