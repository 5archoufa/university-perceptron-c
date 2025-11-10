#include "game/creature/stomach.h"

// ----------------------------------------
// Initialization 
// ----------------------------------------

void Stomach_Init(Stomach *stomach, float starvationRate)
{
    stomach->hunger = 0.0f;
    stomach->starvationRate = starvationRate;
    stomach->feeling_hunger = PhysicalFeeling_Create("Hunger", 0.0);
}