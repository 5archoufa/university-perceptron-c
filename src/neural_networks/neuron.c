#include "neural_networks/neuron.h"
#include <stdlib.h>
#include <stdio.h>
#include "utilities/math/stupid_math.h"
#include "entity/entity.h"

void Neuron_Setup(Neuron *neuron, int weights_size, float (*CaculateInitialBias)(), float (*CaculateInitialWeight)())
{
    // Weights
    neuron->weights_size = weights_size;
    neuron->weights = malloc(weights_size * sizeof(float));
    for (int i = 0; i < weights_size; i++)
    {
        neuron->weights[i] = CaculateInitialWeight();
    }
    // Bias
    neuron->bias = CaculateInitialBias();
}

void Neuron_Free(Neuron *neuron)
{
    free(neuron->weights);
}

void Neuron_Print(Neuron *neuron)
{
    printf("Neuron:\n");
    printf("Weight Count: %d\n", neuron->weights_size);
    printf("Weights: ");
    for (int i = 0; i < neuron->weights_size; i++)
    {
        printf("%f ", neuron->weights[i]);
    }
    printf("\nBias: %f\n", neuron->bias);
}