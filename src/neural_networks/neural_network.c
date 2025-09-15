#include "neural_networks/neural_network.h"
#include <stdlib.h>
#include <stdio.h>

NeuralNetwork *CreateNeuralNetwork(char *name, int layerCount, Layer **layers)
{
    NeuralNetwork *neuralNetwork = malloc(sizeof(NeuralNetwork));
    neuralNetwork->name = name;
    neuralNetwork->layers = layers;
    neuralNetwork->layerCount = layerCount;
    return neuralNetwork;
}

NeuralNetwork *CreateNeuralNetwork_RandomWeights(char* name, int inputCount, int layerCount, int *neuronCounts)
{
    // Layers
    Layer **layers = malloc(layerCount * sizeof(Layer));
    for (int i = layerCount - 1; i >= 0; i--) // Start from the lastLayer;
    {
        char *nameBuffer = malloc(25 * sizeof(char));
        // Layer name
        char code = i == 0? 'I' : i == layerCount - 1? 'O' : 'H';
        sprintf(nameBuffer, "%c%d", code, i + 1);
        printf("%s\n", nameBuffer);
        // Creation
        int weightCount = i > 0? neuronCounts[i - 1] : inputCount;
        layers[i] = CreateLayer_RandomWeights(nameBuffer, neuronCounts[i], weightCount);
    }
    // Assembly
    NeuralNetwork *neuralNetwork = malloc(sizeof(NeuralNetwork));
    neuralNetwork->name = name;
    neuralNetwork->layers = layers;
    neuralNetwork->layerCount = layerCount;
    return neuralNetwork;
}

void FreeNeuralNetwork(NeuralNetwork *neuralNetwork)
{
    char* p = PNeuralNetwork(neuralNetwork);
    printf("Will free %s...\n", p);
    free(p);
    for(int i = 0;i<neuralNetwork->layerCount;i++){
        printf("freeing n%d", i);
        FreeLayer(neuralNetwork->layers[i]);
    }
    free(neuralNetwork->layers);
    free(neuralNetwork);
    printf("Done freeing neural network\n");
}

void PrintNeuralNetwork(NeuralNetwork* NeuralNetwork){
    printf("Neural Network: %s\n", NeuralNetwork->name);
    printf("Layer Count: %d\n", NeuralNetwork->layerCount);
    for(int i = 0;i<NeuralNetwork->layerCount;i++){
        PrintLayer(NeuralNetwork->layers[i]);
    }
}

char* PNeuralNetwork(NeuralNetwork* neuralNetwork){
    char* text = malloc(25 * sizeof(char));
    sprintf(text, "NEURAL_NETWORK<%s>", neuralNetwork->name);
    return text;
}