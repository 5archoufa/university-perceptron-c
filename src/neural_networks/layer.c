#include "neural_networks/layer.h"
#include <stdlib.h>
#include <stdio.h>

void Layer_Setup(Layer *layer, char *name, size_t inputs_size, size_t neurons_size, float (*CaculateInitialBias)(), float (*CaculateInitialWeight)())
{
    layer->name = name;
    layer->inputs_size = inputs_size;
    layer->neurons_size = neurons_size;
    layer->neurons = malloc(neurons_size * sizeof(Neuron));
    for (int i = 0; i < neurons_size; i++)
    {
        Neuron_Setup(&layer->neurons[i], inputs_size, CaculateInitialBias, CaculateInitialWeight);
    }
}

void Layer_Free(Layer *layer)
{
    for (int i = 0; i < layer->neurons_size; i++)
    {
        Neuron_Free(&layer->neurons[i]);
    }
    free(layer->name);
    free(layer->neurons);
}

void Layer_Print(Layer *layer)
{
    char *os = malloc((layer->neurons_size + 1) * sizeof(char));
    for (int i = 0; i < layer->neurons_size; i++)
    {
        os[i] = 'O';
    }
    os[layer->neurons_size] = '\0';
    printf("Layer<%s> [%s]: \n", os, layer->name);
    for (int i = 0; i < layer->neurons_size; i++)
    {
        Neuron_Print(&layer->neurons[i]);
    }
    free(os);
}