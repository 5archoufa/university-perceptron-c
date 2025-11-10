#ifndef STOMACH_H
#define STOMACH_H

// Feelings
#include "game/creature/feelings.h"

// ----------------------------------------
// Types
// ----------------------------------------

typedef struct Stomach
{
    PhysicalFeeling *feeling_hunger;
    float hunger;
    float starvationRate;
} Stomach;

// ----------------------------------------
// Initialization
// ----------------------------------------

void Stomach_Init(Stomach *stomach, float starvationRate);

#endif