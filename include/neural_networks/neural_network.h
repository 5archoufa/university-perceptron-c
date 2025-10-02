#ifndef NEURAL_NETWORK_H
#define NEURAL_NETWORK_H

#include "layer.h"

typedef struct NeuralNetwork{
    char* name;
    int layerCount;
    int inputCount;
    Layer* layers;
} NeuralNetwork;

NeuralNetwork *NeuralNetwork_Create(char *name, int inputCount, int layerCount, int *neuronCounts, float (*CaculateInitialBias)(), float (*CaculateInitialWeight)());
void NeuralNetwork_Free(NeuralNetwork* neuralNetwork);
void NeuralNetwork_Print(NeuralNetwork* NeuralNetwork);

float NN_Bias_Zero();
float NN_Weight_Random();

#endif