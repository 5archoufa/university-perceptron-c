#ifndef NEURON_H
#define NEURON_H

#include <stddef.h>

typedef struct Neuron
{
    size_t weights_size;
    float *weights;
    float bias;
} Neuron;

void Neuron_Setup(Neuron *neuron, int weights_size, float (*CaculateInitialBias)(), float (*CaculateInitialWeight)());
void Neuron_Free(Neuron *neuron);
void Neuron_Print(Neuron *neuron);

#endif