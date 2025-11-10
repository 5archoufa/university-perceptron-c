#include "game/creature/health.h"

// ----------------------------------------
// Initialization 
// ----------------------------------------

void Health_Init(Health* health, float max, float value, float regeneration)
{
    health->max = max;
    health->value = value;
    health->regeneration = regeneration;
}
