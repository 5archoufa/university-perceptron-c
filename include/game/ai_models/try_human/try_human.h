#ifndef TRY_HUMAN_H
#define TRY_HUMAN_H

// Math
#include "utilities/math/v3.h"
// Feelings
#include "game/creature/feelings.h"
// Personality
#include "game/creature/personality.h"

// ----------------------------------------
// Types 
// ----------------------------------------

typedef struct RaycastHit{
    enum{
        RAYCAST_HIT_NONE,
        RAYCAST_HIT_HUMAN,
        RAYCAST_HIT_RABBIT,
        RAYCAST_HIT_TRIGONOID,
    } hitType;
} RaycastHit;

typedef struct AIModel_TryHuman
{
    // ============ Input::Human ============ //
    // Transform
    float rotationY;
    V3 positionInIsland;
    // Sensory Input

    // Feelings
    EmotionalFeelings *feelings;
    // Personality
    Personality *personality;
    // 

} AIModel_TryHuman;

// ----------------------------------------
// Creation & Freeing 
// ----------------------------------------

AIModel_TryHuman *AIModel_TryHuman_Create();
void AIModel_TryHuman_Free(AIModel_TryHuman *tryHuman);

#endif