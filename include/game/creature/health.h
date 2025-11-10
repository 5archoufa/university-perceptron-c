#ifndef HEALTH_H
#define HEALTH_H

// ----------------------------------------
// Types 
// ----------------------------------------

typedef struct Health{
    float value;
    float max;
    float regeneration;
} Health;

// ----------------------------------------
// Initialization 
// ----------------------------------------

void Health_Init(Health* health, float max, float value, float regeneration);

#endif