#include "neuron.h"

typedef struct Layer{
    char* name;
    int neuronCount;
    Neuron** neurons;
} Layer;

Layer* CreateLayer(char* name, int neuronCount, Neuron** neurons);
Layer* CreateLayer_RandomWeights(char* name, int neuronCount, int weightCount);
void FreeLayer(Layer* layer);
void PrintLayer(Layer* layer);