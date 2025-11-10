#include "game/creature/feelings.h"
// C
#include <stdlib.h>
#include <string.h>

// ----------------------------------------
// Initialization
// ----------------------------------------

PhysicalFeeling *PhysicalFeeling_Create(char *name, float initialValue)
{
    PhysicalFeeling *feeling = malloc(sizeof(PhysicalFeeling));
    // Name
    feeling->name = malloc((strlen(name) + 1) * sizeof(char));
    strcpy(feeling->name, name);
    // Value
    feeling->value = initialValue;
    return feeling;
}