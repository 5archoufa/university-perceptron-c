#ifndef EC_NN_LAYER_H
#define EC_NN_LAYER_H

#include "entity/components/neural_network/ec_nn_neuron.h"
#include "neural_networks/neural_network.h"
#include "entity/entity.h"

typedef struct {
    Component* component;
    EC_NN_Neuron **neurons;
    size_t neurons_size;
} EC_NN_Layer;

EC_NN_Layer *Prefab_Layer(Entity *parent, TransformSpace TS, V3 position, Quaternion rotation, V3 scale, Layer *layer);
void EC_Layer_Free(Component *component);
float EC_Layer_GetHeight(size_t neurons_size);

#endif