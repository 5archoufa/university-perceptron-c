#include "game/creature/stomach.h"

// ----------------------------------------
// Initialization & Freeing
// ----------------------------------------

void Stomach_Init(Stomach *stomach, float starvationRate)
{
    stomach->hunger = 0.0f;
    stomach->starvationRate = starvationRate;
    PhysicalFeeling_Init(&stomach->feeling_hunger, "Hunger", 0.0f);
}

void Stomach_Free(Stomach *stomach)
{
}