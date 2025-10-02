#include "neural_networks/neural_network.h"
#include <stdlib.h>
#include <stdio.h>
#include "utilities/math/stupid_math.h"

NeuralNetwork *NeuralNetwork_Create(char *name, int inputCount, int layerCount, int *neuronCounts, float (*CaculateInitialBias)(), float (*CaculateInitialWeight)())
{
    NeuralNetwork *neuralNetwork = malloc(sizeof(NeuralNetwork));
    neuralNetwork->name = name;
    neuralNetwork->layerCount = layerCount;
    neuralNetwork->inputCount = inputCount;
    // Layers
    neuralNetwork->layers = malloc(layerCount * sizeof(Layer));
    int layerNameSize = 5 * sizeof(char);
    char *layerName = malloc(layerNameSize);
    // Hidden Layer 1
    sprintf(layerName, "H1");
    Layer_Setup(&neuralNetwork->layers[0], layerName, inputCount, neuronCounts[0], CaculateInitialBias, CaculateInitialWeight);
    // Other Hidden Layers
    for (int i = 1; i < layerCount - 1; i++) // Start from the lastLayer;
    {
        layerName = malloc(layerNameSize);
        sprintf(layerName, "H%d", i + 1);
        Layer_Setup(&neuralNetwork->layers[i], layerName, neuronCounts[i - 1], neuronCounts[i], CaculateInitialBias, CaculateInitialWeight);
    }
    // Output Layer
    layerName = malloc(layerNameSize);
    sprintf(layerName, "O");
    Layer_Setup(&neuralNetwork->layers[layerCount - 1], layerName, neuronCounts[layerCount - 2], neuronCounts[layerCount - 1], CaculateInitialBias, CaculateInitialWeight);
    return neuralNetwork;
}

void NeuralNetwork_Free(NeuralNetwork *neuralNetwork)
{
    for (int i = 0; i < neuralNetwork->layerCount; i++)
    {
        Layer_Free(&neuralNetwork->layers[i]);
    }
    free(neuralNetwork->layers);
    free(neuralNetwork);
    printf("Done freeing neural network\n");
}

void NeuralNetwork_Print(NeuralNetwork *NeuralNetwork)
{
    printf("Neural Network: %s\n", NeuralNetwork->name);
    printf("Layer Count: %d\n", NeuralNetwork->layerCount);
    for (int i = 0; i < NeuralNetwork->layerCount; i++)
    {
        Layer_Print(&NeuralNetwork->layers[i]);
    }
}

float NN_Bias_Zero()
{
    return 0.0;
}

float NN_Weight_Random()
{
    return RandomFloat(0.0, 1.0);
}