#ifndef SIMPLE_RIG_H
#define SIMPLE_RIG_H

//Entity
#include "entity/transform.h"
// Math
#include "utilities/math/v3.h"

// ------------------------- 
// Types 
// -------------------------

typedef struct SimpleRig_Limb SimpleRig_Limb;

struct SimpleRig_Limb
{
    SimpleRig_Limb *parent;
    Quaternion baseRotation;
    float length;
};

typedef struct SimpleRig{
    SimpleRig_Limb arms;
    SimpleRig_Limb legs;
} SimpleRig;

#endif