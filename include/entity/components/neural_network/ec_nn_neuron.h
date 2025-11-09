#ifndef EC_NN_NEURON_H
#define EC_NN_NEURON_H

#include "neural_networks/neuron.h"
#include "entity/entity.h"
#include "entity/transform.h"

typedef struct EC_NN_Neuron EC_NN_Neuron;

extern const float EC_NEURON_WIDTH;

struct EC_NN_Neuron
{
    Component *component;
    Neuron *neuron;
};

EC_NN_Neuron *Prefab_Neuron(Entity *parent, TransformSpace TS, V3 position, Quaternion rotation, V3 scale, Neuron *neuron);
EC_NN_Neuron *EC_Neuron_Create(Entity *entity, Neuron *neuron);

#endif