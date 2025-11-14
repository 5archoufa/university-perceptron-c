#include "game/creature/feelings.h"
// C
#include <stdlib.h>
#include <string.h>

// ----------------------------------------
// Initialization
// ----------------------------------------

void PhysicalFeeling_Init(PhysicalFeeling *feeling, const char *name, float value)
{
    feeling->name = malloc(strlen(name) + 1);
    strcpy(feeling->name, name);
    feeling->value = value;
}

void PhysicalFeeling_Free(PhysicalFeeling *feeling)
{
    if (feeling == NULL)
        return;
    if (feeling->name != NULL)
    {
        free(feeling->name);
        feeling->name = NULL;
    }
}