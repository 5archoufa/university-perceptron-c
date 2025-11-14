#ifndef FEELINGS_H
#define FEELINGS_H

// ----------------------------------------
// Types 
// ----------------------------------------

typedef struct EmotionalFeelings{
    float happiness;
    float sadness;
    float anger;
    float fear;
    float anxiety;
    float disgust;
    float peace;
    float guilt;
} EmotionalFeelings;

/// @brief Use PhysicalFeeling_Init and PhysicalFeeling_Free to manage memory
typedef struct PhysicalFeeling{
    char *name;
    float value;
} PhysicalFeeling;

// ----------------------------------------
// Physical Feelings
// ----------------------------------------

void PhysicalFeeling_Init(PhysicalFeeling *feeling, const char *name, float value);
void PhysicalFeeling_Free(PhysicalFeeling *feeling);

#endif