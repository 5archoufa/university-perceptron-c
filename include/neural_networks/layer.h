#ifndef LAYER_H
#define LAYER_H

#include <stddef.h>
#include "neuron.h"

typedef struct Layer{
    char* name;
    size_t inputs_size;
    size_t neurons_size;
    Neuron* neurons;
} Layer;

void Layer_Setup(Layer* layer, char* name, size_t inputs_size, size_t neurons_size, float (*CaculateInitialBias)(), float (*CaculateInitialWeight)());
void Layer_Print(Layer* layer);
void Layer_Free(Layer* layer);

#endif