#include "game/creature/personality.h"
// Math
#include "utilities/math/stupid_math.h"

// ----------------------------------------
// Initialization 
// ----------------------------------------

void Personality_InitRandomly(Personality* personality){
    personality->envy = RandomFloat(0.0f, 1.0f);
    personality->gluttony = RandomFloat(0.0f, 1.0f);
    personality->greed = RandomFloat(0.0f, 1.0f);
    personality->lust = RandomFloat(0.0f, 1.0f);
    personality->pride = RandomFloat(0.0f, 1.0f);
    personality->sloth = RandomFloat(0.0f, 1.0f);
    personality->wrath = RandomFloat(0.0f, 1.0f);
}