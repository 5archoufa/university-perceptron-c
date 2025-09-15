#include "layer.h"

typedef struct NeuralNetwork{
    char* name;
    int layerCount;
    Layer** layers;
} NeuralNetwork;

NeuralNetwork* CreateNeuralNetwork(char* name, int layerCount, Layer** layers);
NeuralNetwork *CreateNeuralNetwork_RandomWeights(char* name, int inputCount, int layerCount, int *neuronCounts);
void FreeNeuralNetwork(NeuralNetwork* neuralNetwork);
void PrintNeuralNetwork(NeuralNetwork* NeuralNetwork);
char* PNeuralNetwork(NeuralNetwork* neuralNetwork);