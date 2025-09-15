#include "neural_networks/neuron.h"
#include <stdlib.h>
#include <stdio.h>
#include "utilities/math/stupid_math.h"

Neuron *CreateNeuron(int weightCount, float *weights, int bias)
{
    Neuron *neuron = malloc(sizeof(Neuron));
    neuron->weightCount = weightCount;
    neuron->weights = weights;
    neuron->bias = bias;
    return neuron;
}

Neuron *CreateNeuron_RandomWeights(int weightCount)
{
    // Neuron Allocation
    Neuron *neuron = malloc(sizeof(Neuron));
    // Weights
    neuron->weightCount = weightCount;
    neuron->weights = malloc(weightCount * sizeof(Neuron));
    for (int i = 0; i < weightCount; i++)
    {
        neuron->weights[i] = RandomFloat(0.0, 1.0);
    }
    // Bias
    float bias = RandomFloat(0.0, 1.0);
    neuron->bias =bias;
    // Return
    return neuron;
}

void FreeNeuron(Neuron *neuron)
{
    free(neuron->weights);
    free(neuron);
}

void PrintNeuron(Neuron *neuron)
{
    printf("Neuron:\n");
    printf("Weight Count: %d\n", neuron->weightCount);
    printf("Weights: ");
    for (int i = 0; i < neuron->weightCount; i++)
    {
        printf("%f ", neuron->weights[i]);
    }
    printf("\nBias: %f\n", neuron->bias);
}