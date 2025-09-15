#include "neural_networks/layer.h"
#include <stdlib.h>
#include <stdio.h>

Layer *CreateLayer(char *name, int neuronCount, Neuron **neurons)
{
    Layer *layer = malloc(sizeof(Layer));
    layer->name = name;
    layer->neuronCount = neuronCount;
    layer->neurons = neurons;
    return layer;
}

Layer *CreateLayer_RandomWeights(char *name, int neuronCount, int weightCount)
{
    Layer *layer = malloc(sizeof(Layer));
    layer->name = name;
    layer->neuronCount = neuronCount;
    layer->neurons = malloc(neuronCount * sizeof(Neuron));
    for (int i = 0; i < neuronCount; i++)
    {
        layer->neurons[i] = CreateNeuron_RandomWeights(weightCount);
    }
    return layer;
}

void FreeLayer(Layer *layer)
{
    printf("Free layer %s\n", layer->name);
    for (int i = 0; i < layer->neuronCount; i++)
    {
        FreeNeuron(layer->neurons[i]);
    }
    free(layer->neurons);
    free(layer->name);
    free(layer);
}

void PrintLayer(Layer *layer)
{
    char *os = malloc(layer->neuronCount * sizeof(char));
    for (int i = 0; i < layer->neuronCount; i++)
    {
        os[i] = 'O';
    }
    printf("Layer<%s> [%s]: \n", os, layer->name);
    for (int i = 0; i < layer->neuronCount; i++)
    {
        PrintNeuron(layer->neurons[i]);
    }
    free(os);
}