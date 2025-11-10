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

typedef struct PhysicalFeeling{
    char *name;
    float value;
} PhysicalFeeling;

// ----------------------------------------
// Initialization 
// ----------------------------------------

PhysicalFeeling *PhysicalFeeling_Create(char *name, float initialValue);

#endif