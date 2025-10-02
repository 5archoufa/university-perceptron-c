#ifndef EC_NN_H
#define EC_NN_H

#include "entity/components/neural_network/ec_nn_layer.h"
#include "entity/entity.h"

typedef struct EC_NN EC_NN;

struct EC_NN
{
    Component* component;
    size_t layers_size;
    EC_NN_Layer **EC_layers;
};

EC_NN *Prefab_NeuralNetwork(Entity *parent, V3 position, float rotation, V2 scale, V2 pivot, NeuralNetwork *neuralNetwork);
void EC_NeuralNetwork_Free(Component *component);

#endif