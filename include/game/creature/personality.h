#ifndef PERSONALITY_H
#define PERSONALITY_H

// ----------------------------------------
// Types 
// ----------------------------------------

typedef struct Personality{
    float lust;
    float gluttony;
    float pride;
    float sloth;
    float wrath;
    float greed;
    float envy;
} Personality;

void Personality_InitRandomly(Personality* personality);

#endif